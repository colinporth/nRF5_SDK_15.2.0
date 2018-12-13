//{{{
/** @file
 * @brief LED Button Service Server module.
 * @details This module implements a custom LED Button Service with an LED and Button Characteristics.
 *          During initialization, the module adds the LED Button Service and Characteristics
 *          to the BLE stack database.
 *
 *          The application must supply an event handler for receiving LED Button Service
 *          events. Using this handler, the service notifies the application when the
 *          LED value changes.
 *
 *          The service also provides a function for letting the application notify
 *          the state of the Button Characteristic to connected peers.
 *
 * @note    The application must register this module as BLE event observer using the
 *          NRF_SDH_BLE_OBSERVER macro. Example:
 *          @code
 *              ble_hids_t instance;
 *              NRF_SDH_BLE_OBSERVER(anything, BLE_HIDS_BLE_OBSERVER_PRIO,
 *                                   ble_hids_on_ble_evt, &instance);
 *          @endcode
 */
//}}}
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

/**@brief   Macro for defining a ble_lbs instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_LBS_DEF(_name)  \
  static ble_lbs_t _name;   \
  NRF_SDH_BLE_OBSERVER(_name ## _obs, BLE_LBS_BLE_OBSERVER_PRIO, ble_lbs_on_ble_evt, &_name)

#define LBS_UUID_BASE        {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, \
                              0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}

#define LBS_UUID_SERVICE     0x1523
#define LBS_UUID_BUTTON_CHAR 0x1524
#define LBS_UUID_LED_CHAR    0x1525

typedef struct ble_lbs_s ble_lbs_t;
typedef void (*ble_lbs_led_write_handler_t) (uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t new_state);

typedef struct {
  ble_lbs_led_write_handler_t led_write_handler; /**< Event handler to be called when the LED Characteristic is written. */
  } ble_lbs_init_t;

/**@brief LED Button Service structure. This structure contains various status information for the service. */
struct ble_lbs_s {
  uint16_t                    service_handle;      /**< Handle of LED Button Service (as provided by the BLE stack). */
  ble_gatts_char_handles_t    led_char_handles;    /**< Handles related to the LED Characteristic. */
  ble_gatts_char_handles_t    button_char_handles; /**< Handles related to the Button Characteristic. */
  uint8_t                     uuid_type;           /**< UUID type for the LED Button Service. */
  ble_lbs_led_write_handler_t led_write_handler;   /**< Event handler to be called when the LED Characteristic is written. */
  };

//{{{
/**@brief Function for initializing the LED Button Service.
 * @param[out] p_lbs      LED Button Service structure. This structure must be supplied by
 *                        the application. It is initialized by this function and will later
 *                        be used to identify this particular service instance.
 * @param[in] p_lbs_init  Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was initialized successfully. Otherwise, an error code is returned.
 */
uint32_t ble_lbs_init(ble_lbs_t * p_lbs, const ble_lbs_init_t * p_lbs_init);
//}}}
//{{{
/**@brief Function for handling the application's BLE stack events.
 *
 * @details This function handles all events from the BLE stack that are of interest to the LED Button Service.
 *
 * @param[in] p_ble_evt  Event received from the BLE stack.
 * @param[in] p_context  LED Button Service structure.
 */
void ble_lbs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
//}}}
//{{{
/**@brief Function for sending a button state notification.
 *
 ' @param[in] conn_handle   Handle of the peripheral connection to which the button state notification will be sent.
 * @param[in] p_lbs         LED Button Service structure.
 * @param[in] button_state  New button state.
 *
 * @retval NRF_SUCCESS If the notification was sent successfully. Otherwise, an error code is returned.
 */
uint32_t ble_lbs_on_button_change(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t button_state);
//}}}

//{{{
#ifdef __cplusplus
}
#endif
//}}}
