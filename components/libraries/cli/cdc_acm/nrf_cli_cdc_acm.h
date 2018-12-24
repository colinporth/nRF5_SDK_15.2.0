#pragma once

#include <stdbool.h>
#include "nrf_cli.h"
#include "app_usbd_cdc_acm.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

// Command line interface transport.
extern const nrf_cli_transport_api_t nrf_cli_cdc_acm_transport_api;

// Command line interface class instance.
extern const app_usbd_cdc_acm_t nrf_cli_cdc_acm;

typedef struct {
  nrf_cli_transport_handler_t handler;
  void *                      p_context;
  } nrf_cli_cdc_acm_internal_cb_t;

typedef struct {
  nrf_cli_transport_t             transport;
  nrf_cli_cdc_acm_internal_cb_t * p_cb;
  } nrf_cli_cdc_acm_internal_t;

// CLI USB transport definition
#define NRF_CLI_CDC_ACM_DEF(_name_)                           \
  static nrf_cli_cdc_acm_internal_cb_t CONCAT_2(_name_, _cb); \
  static const nrf_cli_cdc_acm_internal_t _name_ = {          \
    .transport = {.p_api = &nrf_cli_cdc_acm_transport_api},   \
    .p_cb = &CONCAT_2(_name_, _cb),                           \
    }

//{{{
#ifdef __cplusplus
}
#endif
//}}}
