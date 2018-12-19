#pragma once
//{{{  includes
#include <stdbool.h>
#include <stdint.h>
#include <sdk_errors.h>
#include "nrf_section_iter.h"
//}}}

// Power management shutdown types
typedef enum {
  NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF,    // Go to System OFF.
  NRF_PWR_MGMT_SHUTDOWN_STAY_IN_SYSOFF, // Go to System OFF and stay there Useful when battery level is low
  NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU,       // Go to DFU mode.
  NRF_PWR_MGMT_SHUTDOWN_RESET,          // Reset chip.
  NRF_PWR_MGMT_SHUTDOWN_CONTINUE        // Continue shutdown. used by modules that block the shutdown process
  } nrf_pwr_mgmt_shutdown_t;

// Shutdown event types
typedef enum {
  NRF_PWR_MGMT_EVT_PREPARE_WAKEUP = NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF,    //prepare the wakeup mechanism.
  NRF_PWR_MGMT_EVT_PREPARE_SYSOFF = NRF_PWR_MGMT_SHUTDOWN_STAY_IN_SYSOFF, //prepare to stay in System OFF state.
  NRF_PWR_MGMT_EVT_PREPARE_DFU    = NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU,       //prepare to enter DFU mode.
  NRF_PWR_MGMT_EVT_PREPARE_RESET  = NRF_PWR_MGMT_SHUTDOWN_RESET,          //prepare to chip reset.
  } nrf_pwr_mgmt_evt_t;

//{{{
/**@brief Shutdown callback.
 * @param[in] event   Type of shutdown process.
 *
 * @retval    true    System OFF / Enter DFU preparation successful. Process will be continued.
 * @retval    false   System OFF / Enter DFU preparation failed. @ref NRF_PWR_MGMT_SHUTDOWN_CONTINUE
 *                    should be used to continue the shutdown process.
 */
typedef bool (*nrf_pwr_mgmt_shutdown_handler_t)(nrf_pwr_mgmt_evt_t event);
//}}}
//{{{
/**@brief   Macro for registering a shutdown handler. Modules that want to get events
 *          from this module must register the handler using this macro.
 *
 * @details This macro places the handler in a section named "pwr_mgmt_data".
 *
 * @param[in]   _handler    Event handler (@ref nrf_pwr_mgmt_shutdown_handler_t).
 * @param[in]   _priority   Priority of the given handler.
 */
#define NRF_PWR_MGMT_HANDLER_REGISTER(_handler, _priority)                               \
    STATIC_ASSERT(_priority < NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT);               \
    /*lint -esym(528,*_handler_function) -esym(529,*_handler_function) : Symbol not referenced. */         \
    NRF_SECTION_SET_ITEM_REGISTER(pwr_mgmt_data, _priority,                              \
                                  static nrf_pwr_mgmt_shutdown_handler_t const CONCAT_2(_handler, _handler_function)) = (_handler)
//}}}

//{{{
/**@brief   Function for initializing power management.
 *
 * @warning Depending on configuration, this function sets SEVONPEND in System Control Block (SCB).
 *          This operation is unsafe with the SoftDevice from interrupt priority higher than SVC.
 *
 * @retval NRF_SUCCESS
 */
ret_code_t nrf_pwr_mgmt_init();
//}}}
//{{{
/**@brief Function for running power management. Should run in the main loop.
 */
void nrf_pwr_mgmt_run();
//}}}
//{{{
/**@brief Function for indicating activity.
 *
 * @details Call this function whenever doing something that constitutes "activity".
 *          For example, whenever sending data, call this function to indicate that the application
 *          is active and should not disconnect any ongoing communication links.
 */
void nrf_pwr_mgmt_feed();
//}}}
//{{{
/**@brief Function for shutting down the system.
 *
 * @param[in] shutdown_type     Type of operation.
 *
 * @details All callbacks will be executed prior to shutdown.
 */
void nrf_pwr_mgmt_shutdown (nrf_pwr_mgmt_shutdown_t shutdown_type);
//}}}
