/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Device Table plugin.
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

#ifndef SILABS_DEVICE_TABLE_H
#define SILABS_DEVICE_TABLE_H

/**
 * @defgroup device-table Device Table
 * @ingroup component
 * @brief API and Callbacks for the Device Table Component
 *
 * Simple C-based rules engine for demonstrating automatic control through a gateway.
 *
 */

/**
 * @addtogroup device-table
 * @{
 */

typedef uint8_t sl_zigbee_af_plugin_device_table_device_state_t;
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum sl_zigbee_af_plugin_device_table_device_state_t
#else
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
  SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_NULL         = 0x00,
  SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED       = 0x10,
  SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_UNRESPONSIVE = 0x11,
  SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT   = 0x20,
  SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEFT         = 0x30,
  SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN      = 0xff
};

#define SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE 20

typedef struct {
  uint8_t     endpoint;
  uint16_t    deviceId;
  uint32_t    lastMsgTimestamp;
  sl_802154_short_addr_t nodeId;
  sl_802154_long_addr_t  eui64;
  sl_zigbee_af_plugin_device_table_device_state_t state;
  uint16_t clusterIds[SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE];
  uint8_t clusterOutStartPosition;
} sl_zigbee_af_plugin_device_table_entry_t;

#define SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE 250
#define SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID 0xffff
#define SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX 0xffff

/**
 * @name API
 * @{
 */

/** @brief Send a message using the device table.
 *
 * Sends the current framework ZCL command to the device specified
 * by an index.
 *
 * @param index  The index of the device to which to send the CLI command.
 */
void sl_zigbee_af_device_table_cli_index_send(uint16_t index);

/** @brief Send a message using the device table.
 *
 * Sends the current framework ZCL command using a device specified
 * by the EUI64 and endpoint.
 *
 * @param eui64  EUI64 to which to send the CLI command.
 * @param index  Endpoint of the device to which to send the CLI command.
 */
void sl_zigbee_af_device_table_send(sl_802154_long_addr_t eui64, uint8_t endpoint);

/** @brief Send a CLI command based on an index and an endpoint.
 *
 * Sends the current framework ZCL command using a device indexed in
 * the device table.
 *
 * @param index  The index of the device to which to send the command.
 * @param endpoint The endpoint of the device to which to send the command.
 */
void sl_zigbee_af_device_table_cli_index_send_with_endpoint(uint16_t index,
                                                            uint8_t endpoint);

/** @brief Send an internally-generated command.
 *
 * Sends an internally-generated command, such as generated
 * by the sl_zigbee_af_fill_command_on_off_cluster_on(); command.  Note:  this is
 * different from the above send command, which works with the CLI-generated
 * outgoing commands.
 *
 * @param  The index of the device to which to send the command.
 */
void sl_zigbee_af_device_table_command_index_send(uint16_t index);

/** @brief Send an internally-generated command.
 *
 * Sends an internally-generated command, such as generated
 * by the sl_zigbee_af_fill_command_on_off_cluster_on(); command.  Note:  this is
 * different from the above send command, which works with the CLI-generated
 * outgoing commands.
 *
 * @param eui64  EUI64 of the device to which to send the command.
 * @param endpoint  The endpoint of the device to which to send the command.
 */
void sl_zigbee_af_device_table_command_send_with_endpoint(sl_802154_long_addr_t eui64,
                                                          uint8_t endpoint);

/** @brief Send an internally-generated command.
 *
 * Sends an internally-generated command, such as generated
 * by the sl_zigbee_af_fill_command_on_off_cluster_on(); command.  Note:  this is
 * different from the above send command, which works with the CLI-generated
 * outgoing commands.
 *
 * @param index  The index of the device to which to send the command.
 * @param endpoint  The endpoint of the device to which to send the command.
 */
void sl_zigbee_af_device_table_command_index_send_with_endpoint(uint16_t index,
                                                                uint8_t endpoint);

/** @brief Compute the EUI64 from the node ID in the device table.
 *
 * This function looks up the device-table entry using the node ID and
 * return the EUI64.
 *
 * @param sl_zigbee_node_id  The node ID of the device.
 * @param eui64  The location of the EUI64 variable.
 *
 * @return bool  True if an EUI can be found, false if not.
 */
bool sl_zigbee_af_device_table_get_eui64_from_node_id(sl_802154_short_addr_t sl_zigbee_node_id,
                                                      sl_802154_long_addr_t eui64);

/** @brief Compute the node ID from the device table index.
 *
 * Looks up the device table entry based on the index and returns
 * the node ID.
 *
 * @param index  The index of the device.
 *
 * @return  The node ID of the device.
 */
uint16_t sl_zigbee_af_device_table_get_node_id_from_index(uint16_t index);

/** @brief Find the endpoint index from node ID and endpoint number.
 *
 * Computes the index into the endpoint table based on the node ID and the
 * endpoint number.
 *
 * @param sl_zigbee_node_id  The node ID of the device.
 * @param endpoint  The desired endpoint.
 *
 * @return  The index of the device.
 */
uint16_t sl_zigbee_af_device_table_get_endpoint_from_node_id_and_endpoint(sl_802154_short_addr_t sl_zigbee_node_id,
                                                                          uint8_t endpoint);

/** @brief Find the index based on the node ID.
 *
 * Computes the device-table index based on the node ID.
 *
 * @param sl_zigbee_node_id  The node ID of the current device.
 *
 * @return  The index in the device-table.
 */
uint16_t sl_zigbee_af_device_table_get_index_from_node_id(sl_802154_short_addr_t sl_zigbee_node_id);

/** @brief Find the index based on the EUI64.
 *
 * Computes the first device table index from the EUI64.
 *
 * @param eui64  EUI64 of the desired node ID.
 *
 * @return  The first index that matches the EUI64.
 */
uint16_t sl_zigbee_af_device_table_get_first_index_from_eui64(sl_802154_long_addr_t eui64);

/** @brief Find the index based on the EUI64 and endpoint.
 *
 * Computes the node ID based on the device table entry that matches the EUI64.
 *
 * @param eui64  EUI64 of the desired index.
 * @param endpoint  The endpoint for the desired index.
 *
 * @return  Index that matches the EUI64
 */
uint16_t sl_zigbee_af_device_table_get_index_from_eui64_and_endpoint(sl_802154_long_addr_t eui64,
                                                                     uint8_t endpoint);

/** @brief Find the node ID based on the EUI64.
 *
 * Computes the node ID based on the device table entry that matches the EUI64.
 *
 * @param eui64  EUI64 of the desired node ID.
 *
 * @return  The node ID that matches the EUI64.
 */
uint16_t sl_zigbee_af_device_table_get_node_id_from_eui64(sl_802154_long_addr_t eui64);

/** @brief Change the device status based on the last sent message.
 *
 * If the last sent message was a success, change the device state to "joined".
 * However, if the last message sent was not a success, change the device state
 * to unresponsive.
 *
 * @param nodeId  The node ID in the last outgoing message.
 * @param status  sl_status_t of the last outgoing message.
 * @param profileId  The profile ID of the last outgoing message.
 * @param clusterId  The cluster ID of the last outgoing message.
 */
void sl_zigbee_af_device_table_message_sent_status(sl_802154_short_addr_t nodeId,
                                                   sl_status_t status,
                                                   uint16_t profileId,
                                                   uint16_t clusterId);

/** @brief Send a leave message to a device based on the index.
 *
 * Sends a leave message to the device indicated by the index.
 *
 * @param index  The index of the device to whom to send the leave message.
 */
void sl_zigbee_af_device_table_send_leave(uint16_t index);

/** @brief Inform the device table that a message was received.
 *
 * The device table keeps track of whether a device has become unresponsive.
 * Hearing from a known device causes the device table to transition the
 * device state from "unresponsive" to "joined". Hearing from an unknown
 * device causes the device table to discover the device, potentially
 * fixing routing issues (i.e., if the node ID has changed).
 *
 * @param nodeId  The node ID from which the last incoming message came.
 */
void sl_zigbee_af_device_table_message_received(sl_802154_short_addr_t nodeId);

/** @brief Kick off device table route repair.
 *
 * A message was sent to the device and failed. Therefore, the route
 * repair will be attempted and the node marked as unresponsive. Currently, route
 * repair simply sends a broadcast message looking for the node ID based on
 * the EUI64. It may be replaced with something more sophisticated in the
 * future.
 *
 * @param nodeId  The node ID of the newly unresponsive node.
 */
void sl_zigbee_af_device_table_initiate_route_repair(sl_802154_short_addr_t nodeId);

/** @brief Return a pointer to the device table structure.
 *
 * Returns a pointer to the device table structure.
 *
 * @return  A pointer to the device table structure.
 */
sl_zigbee_af_plugin_device_table_entry_t* sl_zigbee_af_device_table_pointer(void);

/** @brief Return a pointer to the device table entry.
 *
 * Returns a pointer to the device table entry based on the device table index.
 *
 * @param index  The index of the device table entry in which we are interested.
 *
 * @return  A pointer to the device table entry.
 */
sl_zigbee_af_plugin_device_table_entry_t* sl_zigbee_af_device_table_find_device_table_entry(uint16_t index);

/** @brief Inform the device table that a new device joined.
 *
 * Call this function in the device table
 * when a device joins, rejoins, or sends an end-device announce.
 * If the device is new, it will
 * generate a new device table entry and kick off the discovery process. If
 * the device is not new, the device-table will make sure the node ID matches
 * the EUI64 and make an update if necessary. It will also check to
 * verify the discovery process has completed and attempt to complete it.
 *
 * @param newNodeId  The node ID of the newly joined device.
 *
 * @param newNodeEui64  The EUI64 of the newly joined device.
 */
void sl_zigbee_af_device_table_new_device_join_handler(sl_802154_short_addr_t newNodeId,
                                                       sl_802154_long_addr_t newNodeEui64);

/** @brief Calculate time since the device heard the last message.
 *
 * Computes the amount of time, in seconds, since receiving a
 * message from the device.
 *
 * @param index  The index of the device.
 *
 * @return  The time since the last message (in seconds).
 */
uint32_t sl_zigbee_af_device_table_time_since_last_message(uint16_t index);

/** @brief Return true if the two EUI64 parameters match.
 *
 * Determines whether two EUI64 values are the same. The function
 * will print a feedback message to indicate if both EUI64 values are NULL.
 * The function will also print out a feedback message to indicate if the EUI64
 * values match but in reverse order.
 *
 * @param eui64a  The first EUI64 to compare.
 * @param eui64b  The second EUI64 to compare.
 */
bool sl_zigbee_af_device_table_match_eui64(sl_802154_long_addr_t a, sl_802154_long_addr_t b);

/** @brief Clear the device table.
 *
 * Clears the device table and the backup file of the device
 * table is also cleared if it is a host.
 *
 */
void sl_zigbee_af_device_table_clear(void);

/** @brief Called when a device has been removed from the device table
 *
 * Called when the device has been removed from the table.
 *
 * @param currentIndex  Index of the removed device.
 */
void sl_zigbee_af_device_table_index_removed_cb(uint16_t currentIndex);

/** @brief Called when the device table has been initialized
 *
 * Called when the device table has been initialized
 *
 */
void sl_zigbee_af_device_table_initialized(void);

/** @} */ // end of name API

// ZigBee device IDs
#define DEVICE_ID_ON_OFF_SWITCH 0x0000
#define DEVICE_ID_LEVEL_CONTROL_SWITCH 0x0001
#define DEVICE_ID_ON_OFF_OUTPUT 0x0002
#define DEVICE_ID_LEVEL_CONTROL_OUTPUT 0x0003
#define DEVICE_ID_SCENE_SELECTOR 0x0004
#define DEVICE_ID_CONFIG_TOOL 0x0005
#define DEVICE_ID_REMOTE_CONTROL 0x0006
#define DEVICE_ID_COMBINED_INTERFACE 0x0007
#define DEVICE_ID_RANGE_EXTENDER 0x0008
#define DEVICE_ID_MAINS_POWER_OUTLET 0x0009
#define DEVICE_ID_DOOR_LOCK 0x000a
#define DEVICE_ID_DOOR_LOCK_CONTROLLER 0x000b
#define DEVICE_ID_SIMPLE_SENSOR 0x000c
#define DEVICE_ID_CONSUMPTION_AWARENESS_DEVICE 0x000d
#define DEVICE_ID_HOME_GATEWAY 0x0050
#define DEVICE_ID_SMART_PLUG 0x0051
#define DEVICE_ID_WHITE_GOODS 0x0052
#define DEVICE_ID_METER_INTERFACE 0x0053

#define DEVICE_ID_ON_OFF_LIGHT 0x0100
#define DEVICE_ID_DIMMABLE_LIGHT 0x0101
#define DEVICE_ID_COLOR_DIMMABLE_LIGHT 0x0102
#define DEVICE_ID_ON_OFF_LIGHT_SWITCH 0x0103
#define DEVICE_ID_DIMMER_SWITCH 0x0104
#define DEVICE_ID_COLOR_DIMMER_SWITCH 0x0105
#define DEVICE_ID_LIGHT_SENSOR 0x0106
#define DEVICE_ID_OCCUPANCY_SENSOR 0x0107

#define DEVICE_ID_SHADE 0x0200
#define DEVICE_ID_SHADE_CONTROLLER 0x0201
#define DEVICE_ID_WINDOW_COVERING_DEVICE 0x0202
#define DEVICE_ID_WINDOW_COVERING_CONTROLLER 0x0203

#define DEVICE_ID_HEATING_COOLING_UNIT 0x0300
#define DEVICE_ID_THERMOSTAT 0x0301
#define DEVICE_ID_TEMPERATURE_SENSOR 0x0302
#define DEVICE_ID_PUMP 0x0303
#define DEVICE_ID_PUMP_CONTROLLER 0x0304
#define DEVICE_ID_PRESSURE_SENSOR 0x0305
#define DEVICE_ID_FLOW_SENSOR 0x0306
#define DEVICE_ID_MINI_SPLIT_AC 0x0307

#define DEVICE_ID_IAS_CIE 0x0400
#define DEVICE_ID_IAS_ANCILLARY_CONTROL 0x0401
#define DEVICE_ID_IAS_ZONE 0x0402
#define DEVICE_ID_IAS_WARNING 0x0403

/** @} */ // end of device-table

#endif //__DEVICE_TABLE_H
