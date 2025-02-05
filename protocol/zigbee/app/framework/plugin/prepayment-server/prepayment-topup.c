/***************************************************************************//**
 * @file
 * @brief Implemented routines for storing prepayment topups.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "prepayment-topup.h"

#include "prepayment-server-config.h"
#include "zap-cluster-command-parser.h"

// This function is called when a top up command is received.  The callback must determine if the top up
// command is valid or not.
//extern bool sl_zigbee_af_prepayment_server_consumer_top_up_cb(uint8_t originatingDevice, uint8_t* topUpCode);

bool consumerTopUpIsValid(uint8_t *topUpCode);
static void print_publish_top_up_payload(sl_zigbee_zcl_top_up_payload_t *ptopUpPayload, uint8_t index);

#define UTRN_HIGH_WORD_BASE_VALUE  0x669D529B
#define UTRN_LOW_WORD_BASE_VALUE   0x714A0000

#define PTUT_HEADER_BITMASK                 0xFE000000      // Subclass should be non-zero
#define PTUT_VALUE_CLASS_BITMASK_ONE_UNIT   0x01800000
#define PTUT_VALUE_BITMASK                  0x007FFC00
#define PTUT_ORIGINATOR_COUNTER_BITMASK     0x000003FF

#define PTUT_VALUE_DOWNSHIFT 10

/***********************************************************************
   NOTES about Consumer Top Up command UTRN:
   GBCS, Chapter 14 - Consumer Top Up UTRN is constructed as:
   aaab bbbc cyyy yyyy yyyy yyzz zzzz zzzz mmmm ... mmmm
   where
   [  ========   Prepayment Top Up Token (PTUT)  ========  ]
   aaa:  3 bits - PTUT Lead (fixed 0b000 value)
   bbbb: 4 bits - PTUT Sub Class (fixed 0b0000 value)
   cc:   2 bits - PTUT Value Class (0b00 = 1/100th of currency unit, 0b01 = 1 currency unit)
   yyy..yy: 13 bits - PTUT Value
   zzz..zz: 10 bits - Truncated Originator Counter
   mmm..mm: 32 bits - Supplier MAC

   first two digits of PPTD ranges from 73 to 96.
   Value ranges from 7,394,156,990,786,306,048 (0x669D529B 714A0000)
   to  0x669D529B 714A0000 + (0x00FF FFFF FFFF FFFF) = 0x679D529B7149FFFF   (=7,466,214,584,824,233,983)
   uint32_t highWord, lowWord;
   uint32_t utrnCounter;

   highWord = (((uint32_t)topUpCode[0]) << 8) + topUpCode[1];
   highWord <<= 16;
   highWord += (((uint32_t)topUpCode[2]) << 8) + topUpCode[3];
   highWord -= UTRN_HIGH_WORD_BASE_VALUE;
   utrnCounter = highWord & PTUT_ORIGINATOR_COUNTER_BITMASK;

   Is UTRN Counter in UTRN counter cache?  (GBCS 14.3.6)
   If not in cache, add UTRN counter to cache - 100 entries, circular buffer (GBCS 14.1).

   Compare the amount with the "MaxCreditPerTopUp" attribute (D.7.2.2.1.12).  Is this amount < the attribute value?
   Compare the total credit (current + top up value) with the "MaxCreditLimit" attribute (D.7.2.2.1.11).  Is this
   amount < the attribute value?
   NOTE: These attribute values depend on the prepayment mode (currency or units), which then depend on the currency
   specified in the price cluster, or the units specified in the metering cluster.


   Low Word is the Supplier MAC.
   lowWord = (((uint32_t)topUpCode[4]) << 8) + topUpCode[5];
   lowWord <<= 16;
   lowWord += (((uint32_t)topUpCode[6]) << 8) + topUpCode[7];
   lowWord -= UTRN_LOW_WORD_BASE_VALUE;

***********************************************************************/

#define TOP_UP_ATTRIBUTE_GROUP_DELTA  0x10

//-----------------------
// ZCL commands callbacks

bool sl_zigbee_af_prepayment_cluster_consumer_top_up_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_consumer_top_up_command_t cmd_data;
  sl_zigbee_af_status_t status;
  uint8_t  endpoint;
  uint16_t attributeId;
  uint32_t topUpDateTime;
  int32_t topUpAmount;
  uint8_t  topUpOriginatingDevice;
  uint8_t  topUpCodeRead[27];
  uint8_t  dataType;
  uint8_t i;

  if (zcl_decode_prepayment_cluster_consumer_top_up_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return false;
  }

  topUpDateTime = sl_zigbee_af_get_current_time();

  sl_zigbee_af_prepayment_cluster_println("RX: Consumer Top Up Callback, time=%d", topUpDateTime);
  endpoint = sl_zigbee_af_current_endpoint();

  if ( !sl_zigbee_af_prepayment_server_consumer_top_up_cb(cmd_data.originatingDevice, cmd_data.topUpCode) ) {
    // TODO:  Do what?  -- Send default response?
    return false;
  }

  // Before updating the Top Up #1 attribute set, push the existing #1,#2,#3,#4 attributes down.
  // So Date/Time#1 becomes Date/Time#2, Amount#1 becomes Amount#2, etc.
  // These are optional attributes, so don't care about the read/write return status.
  //for( i=0; i<4; i++ ){
  for ( i = 4; i > 0; ) {
    i--;
    attributeId = (ZCL_TOP_UP_DATE_TIME_1_ATTRIBUTE_ID + (i * TOP_UP_ATTRIBUTE_GROUP_DELTA) );
    status = sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                                         attributeId, CLUSTER_MASK_SERVER, (uint8_t *)&topUpDateTime, 4, &dataType);
    if ( status == SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
      (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, (attributeId + TOP_UP_ATTRIBUTE_GROUP_DELTA),
                                          CLUSTER_MASK_SERVER, (uint8_t *)&topUpDateTime, dataType);
    }

    attributeId = (ZCL_TOP_UP_AMOUNT_1_ATTRIBUTE_ID + (i * TOP_UP_ATTRIBUTE_GROUP_DELTA) );
    status = sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                                         attributeId, CLUSTER_MASK_SERVER, (uint8_t *)&topUpAmount, 4, &dataType);
    if ( status == SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
      (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, (attributeId + TOP_UP_ATTRIBUTE_GROUP_DELTA),
                                          CLUSTER_MASK_SERVER, (uint8_t *)&topUpAmount, dataType);
    }

    attributeId = (ZCL_TOP_UP_ORIGINATING_DEVICE_1_ATTRIBUTE_ID + (i * TOP_UP_ATTRIBUTE_GROUP_DELTA) );
    status = sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                                         attributeId, CLUSTER_MASK_SERVER, &topUpOriginatingDevice, 1, &dataType);
    if ( status == SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
      (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, (attributeId + TOP_UP_ATTRIBUTE_GROUP_DELTA),
                                          CLUSTER_MASK_SERVER, &topUpOriginatingDevice, dataType);
    }

    attributeId = (ZCL_TOP_UP_CODE_1_ATTRIBUTE_ID + (i * TOP_UP_ATTRIBUTE_GROUP_DELTA) );
    status = sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                                         attributeId, CLUSTER_MASK_SERVER, topUpCodeRead, 26, &dataType);
    if ( status == SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
      status = sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, (attributeId + TOP_UP_ATTRIBUTE_GROUP_DELTA),
                                            CLUSTER_MASK_SERVER, topUpCodeRead, dataType);
    }
  }

  // Now write the #1 attribute set with values from the top up command.

  topUpDateTime = sl_zigbee_af_get_current_time();
  status = sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_TOP_UP_DATE_TIME_1_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER, (uint8_t *)&topUpDateTime, ZCL_UTC_TIME_ATTRIBUTE_TYPE);

  status = sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_TOP_UP_ORIGINATING_DEVICE_1_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER, &cmd_data.originatingDevice, ZCL_ENUM8_ATTRIBUTE_TYPE);

  status = sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_TOP_UP_CODE_1_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER, cmd_data.topUpCode, ZCL_OCTET_STRING_ATTRIBUTE_TYPE);

// TODO:   How to extract the amount from the Top Up command??

//  topUpAmount = (((uint32_t)topUpCode[0]) << 8) + topUpCode[1];
//  topUpAmount <<= 16;
//  topUpAmount += (((uint32_t)topUpCode[2]) << 8) + topUpCode[3];
//  topUpAmount -= UTRN_HIGH_WORD_BASE_VALUE;
// topUpAmount &= PTUT_VALUE_BITMASK;
// topUpAmount >>= PTUT_VALUE_DOWNSHIFT;
// status = sl_zigbee_af_write_attribute( endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_TOP_UP_AMOUNT_1_ATTRIBUTE_ID,
//                                 CLUSTER_MASK_SERVER, topUpAmount, ZCL_INT32S_ATTRIBUTE_TYPE );

  return true;
}

#define MAX_TOP_UPS   5

bool sl_zigbee_af_prepayment_cluster_get_top_up_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_get_top_up_log_command_t cmd_data;
  sl_802154_short_addr_t nodeId;
  uint8_t srcEndpoint, dstEndpoint;

  if (zcl_decode_prepayment_cluster_get_top_up_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return false;
  }

  if ( cmd_data.numberOfRecords == 0 ) {
    // Requesting maximum possible.
    cmd_data.numberOfRecords = MAX_TOP_UPS;
  }

  sl_zigbee_af_prepayment_cluster_println("RX: GetTopUpLog, endTime=0x%4x, numRec=%d", cmd_data.latestEndTime, cmd_data.numberOfRecords);

  nodeId = sl_zigbee_af_current_command()->source;
  srcEndpoint = sl_zigbee_af_get_command_aps_frame()->destinationEndpoint;
  dstEndpoint = sl_zigbee_af_get_command_aps_frame()->sourceEndpoint;

  sl_zigbee_af_send_publish_top_up_log(nodeId, srcEndpoint, dstEndpoint, cmd_data.latestEndTime, cmd_data.numberOfRecords);

  return true;
}

static uint8_t NumTopUpPayloads;
#define CMD_INDEX_LAST_CMD  0xFE

void sl_zigbee_af_send_publish_top_up_log(sl_802154_short_addr_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                          uint32_t latestEndTime, uint8_t numberOfRecords)
{
  // Figure out how many records to send, then create
  // sl_zigbee_zcl_top_up_payload_t structure array and populate response.
  // Then call sl_zigbee_af_prepayment_server_publish_top_up_log().
  sl_zigbee_af_status_t status;
  sl_zigbee_zcl_top_up_payload_t topUpPayloads[MAX_TOP_UPS];
  uint8_t topUpCodes[27 * MAX_TOP_UPS] = { 0 };
  uint32_t topUpAmount;
  uint32_t topUpTime;
  uint16_t attributeId;
  uint8_t *ptopUpCode;

  uint8_t i;
  uint8_t matchCnt = 0;

  if ( numberOfRecords == 0 ) {
    numberOfRecords = MAX_TOP_UPS;
  }

  sl_zigbee_af_prepayment_cluster_println("Send Publish Top Up Log, %d entries", numberOfRecords);
  ptopUpCode = topUpCodes;

  for ( i = 0; i < MAX_TOP_UPS; i++ ) {
    // Read TOP UP CODE string, TopUpAmount U32, TopUpTime UTC_Time.
    attributeId = ZCL_TOP_UP_DATE_TIME_1_ATTRIBUTE_ID + (i * 0x10);
    status = sl_zigbee_af_read_attribute(srcEndpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId,
                                         CLUSTER_MASK_SERVER, (uint8_t *)&topUpTime, 4, NULL);

    if ( (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) && (topUpTime <= latestEndTime) ) {
      attributeId = ZCL_TOP_UP_CODE_1_ATTRIBUTE_ID + (i * 0x10);
      status = sl_zigbee_af_read_attribute(srcEndpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId,
                                           CLUSTER_MASK_SERVER, ptopUpCode, 26, NULL);

      attributeId = ZCL_TOP_UP_AMOUNT_1_ATTRIBUTE_ID + (i * 0x10);
      status |= sl_zigbee_af_read_attribute(srcEndpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId,
                                            CLUSTER_MASK_SERVER, (uint8_t *)&topUpAmount, 4, NULL);

      if ( status == SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
//        (void) sl_zigbee_af_put_string_in_resp( topUpCode );
//        (void) sl_zigbee_af_put_int32u_in_resp( topUpAmount );
//        (void) sl_zigbee_af_put_int32u_in_resp( topUpTime );
        topUpPayloads[matchCnt].topUpCode = ptopUpCode;
        topUpPayloads[matchCnt].topUpAmount = topUpAmount;
        topUpPayloads[matchCnt].topUpTime = topUpTime;
        matchCnt++;
        ptopUpCode += 26;
        if ( matchCnt >= numberOfRecords ) {
          break;
        }
      } else {
        // failure status - abort since higher groups of these attributes are also not supported.
        break;
      }
    }
  }
  NumTopUpPayloads = matchCnt;
  if ( matchCnt > 0 ) {
    // Command is now ready to be sent.
    sl_zigbee_af_prepayment_server_publish_top_up_log(nodeId, srcEndpoint, dstEndpoint, CMD_INDEX_LAST_CMD, 1, topUpPayloads);
  } else {
    sl_zigbee_af_prepayment_cluster_println("No Matching Top Up Log Entries");
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
  }
}

static void print_publish_top_up_payload(sl_zigbee_zcl_top_up_payload_t *ptopUpPayload, uint8_t index)
{
  sl_zigbee_af_prepayment_cluster_println("= Top Up Payload %d", index);
  sl_zigbee_af_prepayment_cluster_println("  Code=%s", ptopUpPayload->topUpCode);
  sl_zigbee_af_prepayment_cluster_println("  Amount=%d", ptopUpPayload->topUpAmount);
  sl_zigbee_af_prepayment_cluster_println("  Time=%d", ptopUpPayload->topUpTime);
}

// Called when a Get Top Up Log command is received from a prepayment client, or
// when a new top up is performed.
// NOTE:  sl_zigbee_zcl_top_up_payload_t is a struct type.  See se-meter-gas/gen/simulation/af-structs.h
void sl_zigbee_af_prepayment_server_publish_top_up_log(sl_802154_short_addr_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                       uint8_t commandIndex, uint8_t totalNumberOfCommands,
                                                       sl_zigbee_zcl_top_up_payload_t *topUpPayload)
{
  sl_status_t status;
  uint8_t i;

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  // cmdIndex=0xFE, totalNumCmds=1.   APS Fragmentation will deal with message fragmenting if necessary.
  (void) sl_zigbee_af_fill_external_buffer( (ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                            ZCL_PREPAYMENT_CLUSTER_ID,
                                            ZCL_PUBLISH_TOP_UP_LOG_COMMAND_ID,
                                            "uu",
                                            commandIndex, totalNumberOfCommands);

  for ( i = 0; i < NumTopUpPayloads; i++ ) {
    (void) sl_zigbee_af_put_string_in_resp(topUpPayload[i].topUpCode);
    (void) sl_zigbee_af_put_int32u_in_resp(topUpPayload[i].topUpAmount);
    (void) sl_zigbee_af_put_int32u_in_resp(topUpPayload[i].topUpTime);
    print_publish_top_up_payload(&topUpPayload[i], i);
  }

  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if ( status == SL_STATUS_OK ) {
    sl_zigbee_af_prepayment_cluster_println("Sent Publish Top Up Log, %d top ups", NumTopUpPayloads);
  } else {
    sl_zigbee_af_prepayment_cluster_println("Error in Publish Top Up Log %x", status);
  }
}
