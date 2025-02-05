/***************************************************************************//**
 * @file
 * @brief CLI for the Simple Metering Server plugin.
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
#include "simple-metering-server.h"
#include "simple-metering-test.h"
#include  "sl_cli.h"

static uint8_t getEndpointArgument(sl_cli_command_arg_t *arguments, uint8_t index)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, index);
  return (endpoint == 0
          ? sl_zigbee_af_primary_endpoint_for_current_network_index()
          : endpoint);
}

// plugin simple-metering-server print
void sli_zigbee_af_simple_metering_server_cli_print(sl_cli_command_arg_t *arguments)
{
  afTestMeterPrint();
}

// plugin simple-metering-server rate <int:2>
void sli_zigbee_af_simple_metering_server_cli_rate(sl_cli_command_arg_t *arguments)
{
  afTestMeterSetConsumptionRate(sl_cli_get_argument_uint16(arguments, 0),
                                sl_cli_get_argument_uint8(arguments, 1));
}

// plugin simple-metering-server variance <int:2>
void sli_zigbee_af_simple_metering_server_cli_variance(sl_cli_command_arg_t *arguments)
{
  afTestMeterSetConsumptionVariance(sl_cli_get_argument_uint16(arguments, 0));
}

// plugin simple-metering-server adjust <endpoint:1>
void sli_zigbee_af_simple_metering_server_cli_adjust(sl_cli_command_arg_t *arguments)
{
  afTestMeterAdjust(getEndpointArgument(arguments, 0));
}

// plugin simple-metering-server off <endpoint: 1>
void sli_zigbee_af_simple_metering_server_cli_off(sl_cli_command_arg_t *arguments)
{
  afTestMeterMode(getEndpointArgument(arguments, 0), 0);
}

// plugin simple-metering-server electric <endpoint:1>
void sli_zigbee_af_simple_metering_server_cli_electric(sl_cli_command_arg_t *arguments)
{
#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_TYPE == SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_ELECTRIC_METER)
  afTestMeterMode(getEndpointArgument(arguments, 0), 1);
#else
  sl_zigbee_af_core_println("Not applicable for a non Electric Meter.");
#endif
}

// plugin simple-metering-server gas <endpoint:1>
void sli_zigbee_af_simple_metering_server_cli_gas(sl_cli_command_arg_t *arguments)
{
//#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_TYPE == SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_GAS_METER)
  afTestMeterMode(getEndpointArgument(arguments, 0), 2);
// #else
//   sl_zigbee_af_core_println("Not applicable for a non Gas Meter.");
// #endif
}

#ifdef TEST_METER_ERRORS
// plugin simple-metering-server rnd_error <data:1>
void sli_zigbee_af_simple_metering_server_cli_random_error(sl_cli_command_arg_t *arguments)
{
  // enables random error setting at each tick
  afTestMeterRandomError(sl_cli_get_argument_uint8(arguments, 0));
}

// plugin simple-metering-server set_error <data:1> <endpoint:1>
void sli_zigbee_af_simple_metering_server_cli_set_error(sl_cli_command_arg_t *arguments)
{
  // sets error, in the process overriding random_error
  afTestMeterSetError(getEndpointArgument(arguments, 1),
                      sl_cli_get_argument_uint8(arguments, 0));
}
#else // !TEST_METER_ERRORS -- defining stubs
// plugin simple-metering-server rnd_error <data:1>
void sli_zigbee_af_simple_metering_server_cli_random_error(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
}

// plugin simple-metering-server set_error <data:1> <endpoint:1>
void sli_zigbee_af_simple_metering_server_cli_set_error(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
}
#endif // TEST_METER_ERRORS

// plugin simple-metering-server profiles <data:1>
void sli_zigbee_af_simple_metering_server_cli_profiles(sl_cli_command_arg_t *arguments)
{
#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  afTestMeterEnableProfiles(sl_cli_get_argument_uint8(arguments, 0));
#else
  sl_zigbee_af_core_println("Not applicable for 0 configured profiles.");
#endif
}
