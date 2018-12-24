#pragma once
//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

#include "nrf_cli.h"
#include "ble.h"
#include "app_timer.h"
#include "nrf_ringbuf.h"
#include "nordic_common.h"
#include "nrf_ble_gatt.h"

ret_code_t nrf_cli_ble_uart_service_init(void);

extern const nrf_cli_transport_api_t nrf_cli_ble_uart_transport_api;

typedef struct nrf_cli_ble_uart_internal_s nrf_cli_ble_uart_internal_t;

typedef struct {
  nrf_cli_transport_handler_t   handler;
  void *                        p_context;
  uint16_t                      conn_handle;
  bool                          timer_created;
  bool                          service_started;
  } nrf_cli_ble_uart_internal_cb_t;

struct nrf_cli_ble_uart_internal_s {
  nrf_cli_transport_t         transport;
  nrf_cli_ble_uart_internal_cb_t * p_cb;
  app_timer_id_t const *      p_timer;
  nrf_ringbuf_t const * p_rx_ringbuf;
  nrf_ringbuf_t const * p_tx_ringbuf;
  nrf_ble_gatt_t const * p_gatt;
  };

typedef struct {
  uint16_t conn_handle;
  } nrf_cli_ble_uart_config_t;

/**@brief CLI Bluetooth transport definition.
 * @param _name      Name of the instance.
 * @param _p_gatt    Pointer to the nrf_ble_gatt module.
 * @param _tx_buf_sz Size of TX ring buffer.
 * @param _rx_buf_sz Size of RX ring buffer.
 */
#define NRF_CLI_BLE_UART_DEF(_name, _p_gatt, _tx_buf_sz, _rx_buf_sz)   \
    APP_TIMER_DEF(CONCAT_2(_name, _timer));                            \
    NRF_RINGBUF_DEF(CONCAT_2(_name,_tx_ringbuf), _tx_buf_sz);          \
    NRF_RINGBUF_DEF(CONCAT_2(_name,_rx_ringbuf), _rx_buf_sz);          \
    static nrf_cli_ble_uart_internal_cb_t CONCAT_2(_name, _cb);        \
    static const nrf_cli_ble_uart_internal_t _name = {                 \
        .transport = {.p_api = &nrf_cli_ble_uart_transport_api},       \
        .p_cb = &CONCAT_2(_name, _cb),                                 \
        .p_timer = &CONCAT_2(_name, _timer),                           \
        .p_rx_ringbuf = &CONCAT_2(_name,_rx_ringbuf),                  \
        .p_tx_ringbuf = &CONCAT_2(_name,_tx_ringbuf),                  \
        .p_gatt       = _p_gatt,                                       \
    }

//{{{
#ifdef __cplusplus
}
#endif
//}}}
