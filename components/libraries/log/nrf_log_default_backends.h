#pragma once

#include <stdbool.h>
#include "sdk_config.h"
#include "sdk_errors.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

#if NRF_LOG_ENABLED
  #define NRF_LOG_DEFAULT_BACKENDS_INIT() nrf_log_default_backends_init()
#else
  #define NRF_LOG_DEFAULT_BACKENDS_INIT()
#endif

void nrf_log_default_backends_init();

//{{{
#ifdef __cplusplus
}
#endif
//}}}
