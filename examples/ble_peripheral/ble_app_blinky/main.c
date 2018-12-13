//{{{  includes
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"

#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"

#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"

#include "boards.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_lbs.h"

#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "sx1509b.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//}}}
//{{{  defines
#define DEVICE_NAME  "Nordic_Blinky"  // Name of device. Will be included in the advertising data. */

#define APP_BLE_OBSERVER_PRIO  3  // Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG   1  // A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_INTERVAL  64                                      // The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */
#define APP_ADV_DURATION  BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   // The advertising time-out (in units of seconds). When set to 0, we will never time out. */

#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS)  // Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS)  // Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY     0
#define CONN_SUP_TIMEOUT  MSEC_TO_UNITS(4000, UNIT_10_MS)   // Connection supervisory time-out (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)  // Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)   // Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3   // Number of attempts before giving up the connection parameter negotiation. */

#define BUTTON_DETECTION_DELAY  APP_TIMER_TICKS(50)  // Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF  0xDEADBEEF  // Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
//}}}
#define ADVERTISING_LED   BSP_BOARD_LED_0 // Is on when device is advertising
#define CONNECTED_LED     BSP_BOARD_LED_1 // Is on when device has connected
#define LEDBUTTON_LED     BSP_BOARD_LED_2 // LED to be toggled with the help of the LED Button Service
#define LEDBUTTON_BUTTON  BUTTON          // Button that will trigger the notification event with the LED Button Service

BLE_LBS_DEF (gLbs);       // LED Button Service instance
NRF_BLE_QWR_DEF (gQwr);   // Queued Write context
NRF_BLE_GATT_DEF (gGatt); // GATT module instance

static uint16_t gConnHandle = BLE_CONN_HANDLE_INVALID;              // Handle current connection
static uint8_t gAdvHandle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;         // Advertising handle
static uint8_t mEncHandle[BLE_GAP_ADV_SET_DATA_SIZE_MAX];           // Buffer for storing an encoded advertising set
static uint8_t gEncScanResponseData[BLE_GAP_ADV_SET_DATA_SIZE_MAX]; // Buffer for storing an encoded scan data

//{{{
// Struct that contains pointers to the encoded advertising data
static ble_gap_adv_data_t m_adv_data = {
  .adv_data = {
    .p_data = mEncHandle,
    .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
  .scan_rsp_data = {
    .p_data = gEncScanResponseData,
    .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    }
  };
//}}}

//{{{
/**@brief Function for assert macro callback.
 * @details This function will be called in case of an assert in the SoftDevice.
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback (uint16_t line_num, const uint8_t* p_file_name) {
  app_error_handler (DEAD_BEEF, line_num, p_file_name);
  }
//}}}

//{{{
static void advertisingStart() {

  NRF_LOG_INFO ("advertising_start");

  APP_ERROR_CHECK (sd_ble_gap_adv_start (gAdvHandle, APP_BLE_CONN_CFG_TAG));
  bsp_board_led_on (ADVERTISING_LED);
  }
//}}}

//{{{
static void buttonHandler (uint8_t pin_no, uint8_t button_action) {

  switch (pin_no) {
    case LEDBUTTON_BUTTON: {
      NRF_LOG_INFO ("Send button state change");
      ret_code_t err_code = ble_lbs_on_button_change (gConnHandle, &gLbs, button_action);
      if (err_code != NRF_SUCCESS &&
          err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
          err_code != NRF_ERROR_INVALID_STATE &&
          err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING) {
        APP_ERROR_CHECK (err_code);
        }
      }
      break;

    default:
      APP_ERROR_HANDLER (pin_no);
      break;
    }
  }
//}}}

//{{{
static void bleEvtHandler (ble_evt_t const* ble_evt, void* p_context) {

  NRF_LOG_INFO ("bleEvtHandler %d", ble_evt->header.evt_id);

  switch (ble_evt->header.evt_id) {
    //{{{
    case BLE_GAP_EVT_CONNECTED:
      NRF_LOG_INFO ("Connected");

      bsp_board_led_on (CONNECTED_LED);
      bsp_board_led_off (ADVERTISING_LED);

      gConnHandle = ble_evt->evt.gap_evt.conn_handle;
      APP_ERROR_CHECK (nrf_ble_qwr_conn_handle_assign (&gQwr, gConnHandle));

      APP_ERROR_CHECK (app_button_enable());
      break;
    //}}}
    //{{{
    case BLE_GAP_EVT_DISCONNECTED:
      NRF_LOG_INFO ("Disconnected");

      bsp_board_led_off (CONNECTED_LED);
      gConnHandle = BLE_CONN_HANDLE_INVALID;

      APP_ERROR_CHECK (app_button_disable());

      advertisingStart();

      break;
    //}}}
    //{{{
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
      // Pairing not supported
      APP_ERROR_CHECK (sd_ble_gap_sec_params_reply (gConnHandle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL));
      break;
    //}}}
    //{{{
    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
      NRF_LOG_DEBUG ("PHY update request.");

      ble_gap_phys_t const phys = {
        .rx_phys = BLE_GAP_PHY_AUTO,
        .tx_phys = BLE_GAP_PHY_AUTO,
        };
      APP_ERROR_CHECK (sd_ble_gap_phy_update (ble_evt->evt.gap_evt.conn_handle, &phys));
      }
      break;
    //}}}

    //{{{
    case BLE_GATTC_EVT_TIMEOUT:
      // Disconnect on GATT Client timeout event.
      NRF_LOG_DEBUG ("GATT Client Timeout");
      APP_ERROR_CHECK (sd_ble_gap_disconnect (ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
      break;
    //}}}

    //{{{
    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
      // No system attributes have been stored.
      APP_ERROR_CHECK (sd_ble_gatts_sys_attr_set(gConnHandle, NULL, 0, 0));
      break;
    //}}}
    //{{{
    case BLE_GATTS_EVT_TIMEOUT:
      // Disconnect on GATT Server timeout event.
      NRF_LOG_DEBUG ("GATT Server Timeout");
      APP_ERROR_CHECK (sd_ble_gap_disconnect (ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
      break;
    //}}}
    default: break;
    }
  }
//}}}
//{{{
static void bleStackInit() {

  APP_ERROR_CHECK (nrf_sdh_enable_request());

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  APP_ERROR_CHECK (nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start));

  // Enable BLE stack.
  APP_ERROR_CHECK (nrf_sdh_ble_enable (&ram_start));

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER (m_ble_observer, APP_BLE_OBSERVER_PRIO, bleEvtHandler, NULL);
  }
//}}}

//{{{
// Function for the GAP initialization.
// This function sets up all the necessary GAP (Generic Access Profile) parameters of the
// device including the device name, appearance, and the preferred connection parameters
static void gapParamsInit() {

  ble_gap_conn_sec_mode_t secMode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN (&secMode);
  APP_ERROR_CHECK (sd_ble_gap_device_name_set (&secMode, (const uint8_t*)DEVICE_NAME, strlen(DEVICE_NAME)));

  ble_gap_conn_params_t gapConnParams;
  memset (&gapConnParams, 0, sizeof(gapConnParams));

  gapConnParams.min_conn_interval = MIN_CONN_INTERVAL;
  gapConnParams.max_conn_interval = MAX_CONN_INTERVAL;
  gapConnParams.slave_latency     = SLAVE_LATENCY;
  gapConnParams.conn_sup_timeout  = CONN_SUP_TIMEOUT;

  APP_ERROR_CHECK (sd_ble_gap_ppcp_set (&gapConnParams));
  }
//}}}

//{{{
static void nrfQwrErrorHandler (uint32_t nrf_error) {
  APP_ERROR_HANDLER (nrf_error);
  }
//}}}
//{{{
static void ledHandler (uint16_t conn_handle, ble_lbs_t* lbs, uint8_t led_state) {

  if (led_state) {
    NRF_LOG_INFO ("LED ON");
    bsp_board_led_on (LEDBUTTON_LED);
    }
  else {
    NRF_LOG_INFO ("LED OFF");
    bsp_board_led_off (LEDBUTTON_LED);
    }
  }
//}}}
//{{{
static void servicesInit() {

  // init Queued Write Module.
  nrf_ble_qwr_init_t qwr_init = {0};
  qwr_init.error_handler = nrfQwrErrorHandler;
  APP_ERROR_CHECK (nrf_ble_qwr_init (&gQwr, &qwr_init));

  // inite LBS.
  ble_lbs_init_t init = {0};
  init.led_write_handler = ledHandler;
  APP_ERROR_CHECK (ble_lbs_init (&gLbs, &init));
  }
//}}}

//{{{
/**@brief Function for initializing the Advertising functionality.
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertisingInit() {

  ble_uuid_t adv_uuids[] = { {LBS_UUID_SERVICE, gLbs.uuid_type} };

  // Build and set advertising data.
  ble_advdata_t advdata;
  memset(&advdata, 0, sizeof(advdata));
  advdata.name_type          = BLE_ADVDATA_FULL_NAME;
  advdata.include_appearance = true;
  advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

  ble_advdata_t srdata;
  memset(&srdata, 0, sizeof(srdata));
  srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
  srdata.uuids_complete.p_uuids  = adv_uuids;

  APP_ERROR_CHECK (ble_advdata_encode (&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len));
  APP_ERROR_CHECK (ble_advdata_encode (&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len));

  // Set advertising parameters.
  ble_gap_adv_params_t adv_params;
  memset (&adv_params, 0, sizeof(adv_params));

  adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;
  adv_params.duration        = APP_ADV_DURATION;
  adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
  adv_params.p_peer_addr     = NULL;
  adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
  adv_params.interval        = APP_ADV_INTERVAL;

  APP_ERROR_CHECK (sd_ble_gap_adv_set_configure (&gAdvHandle, &m_adv_data, &adv_params));
  }
//}}}

//{{{
// Function for handling the Connection Parameters Module.
// This function will be called for all events in the Connection Parameters Module that are passed to the application.
// All this function does is to disconnect. This could have been done by simply
// setting the disconnect_on_fail config parameter, but instead we use the event handler mechanism to demonstrate its use.
static void onConnParamsEvt (ble_conn_params_evt_t* evt) {

  NRF_LOG_INFO ("onConnParamsEvt %d", evt->evt_type);

  if (evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    APP_ERROR_CHECK (sd_ble_gap_disconnect (gConnHandle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE));
  }
//}}}
//{{{
static void connParamsErrorHandler (uint32_t nrf_error) {
  APP_ERROR_HANDLER (nrf_error);
  }
//}}}
//{{{
static void connParamsInit() {

  ble_conn_params_init_t cp_init;
  memset (&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params                  = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail             = false;
  cp_init.evt_handler                    = onConnParamsEvt;
  cp_init.error_handler                  = connParamsErrorHandler;

  APP_ERROR_CHECK (ble_conn_params_init (&cp_init));
  }
//}}}

int main() {

  APP_ERROR_CHECK (NRF_LOG_INIT (NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  NRF_LOG_INFO ("BlinkyBle "__TIME__" "__DATE__);

  bsp_board_init (BSP_INIT_LEDS);
  APP_ERROR_CHECK (app_timer_init());
  static app_button_cfg_t buttons[] = { {LEDBUTTON_BUTTON, false, BUTTON_PULL, buttonHandler} };
  APP_ERROR_CHECK (app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY));
  APP_ERROR_CHECK (nrf_pwr_mgmt_init());

  bleStackInit();
  gapParamsInit();
  APP_ERROR_CHECK (nrf_ble_gatt_init (&gGatt, NULL));
  servicesInit();
  advertisingInit();
  connParamsInit();

  advertisingStart();

  for (;;)
    if (NRF_LOG_PROCESS() == false)
      nrf_pwr_mgmt_run();
  }
