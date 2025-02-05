/***************************************************************************//**
 * @file sl_zigbee_alternate_mac_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_alternate_mac' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_alternate_mac.h.  Do not manually edit
#include "stack/include/sl_zigbee_alternate_mac.h"
#include "stack/internal/inc/sl_zigbee_alternate_mac_internal_def.h"

sl_zigbee_mac_interface_id_t sl_zigbee_alt_mac_init(const sl_zigbee_alt_mac_config_t *config)
{
  return sli_zigbee_stack_alt_mac_init(config);
}
