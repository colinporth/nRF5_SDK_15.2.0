#pragma once
//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

#include "nrf_cli.h"
#include "nrf_libuarte.h"
#include "nrf_ringbuf.h"
#include "app_timer.h"

extern const nrf_cli_transport_api_t cli_libuarte_transport_api;

typedef struct cli_libuarte_internal_s cli_libuarte_internal_t;

typedef struct {
  nrf_cli_transport_handler_t   handler;
  void *                        p_context;
  bool                          blocking;
  } cli_libuarte_internal_cb_t;

struct cli_libuarte_internal_s {
  nrf_cli_transport_t      transport;
  cli_libuarte_internal_cb_t * p_cb;
  nrf_ringbuf_t const *    p_rx_ringbuf;
  nrf_ringbuf_t const *    p_tx_ringbuf;
  };

typedef struct {
  uint32_t             tx_pin;
  uint32_t             rx_pin;
  nrf_uarte_hwfc_t     hwfc;          ///< Flow control configuration.
  nrf_uarte_parity_t   parity;        ///< Parity configuration.
  nrf_uarte_baudrate_t baudrate;      ///< Baudrate.
  } cli_libuarte_config_t;

#define NRF_CLI_LIBUARTE_DEF(_name, _tx_buf_sz, _rx_buf_sz)   \
  NRF_RINGBUF_DEF(CONCAT_2(_name,_tx_ringbuf), _tx_buf_sz); \
  NRF_RINGBUF_DEF(CONCAT_2(_name,_rx_ringbuf), _rx_buf_sz); \
  static cli_libuarte_internal_cb_t CONCAT_2(_name, _cb);   \
  static const cli_libuarte_internal_t _name = {            \
    .transport = {.p_api = &cli_libuarte_transport_api},  \
    .p_cb = &CONCAT_2(_name, _cb),                        \
    .p_rx_ringbuf = &CONCAT_2(_name,_rx_ringbuf),         \
    .p_tx_ringbuf = &CONCAT_2(_name,_tx_ringbuf),         \
    }

//{{{
#ifdef __cplusplus
}
#endif
//}}}
