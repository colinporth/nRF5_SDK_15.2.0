#pragma once

#include "sdk_config.h"
#include "app_util.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

#define STACK_SIZE          (((int32_t)(STACK_TOP)) - (int32_t)((void *)(STACK_BASE)))

#if NRF_STACK_GUARD_ENABLED
  #define STACK_GUARD_SIZE    (1ul << NRF_STACK_GUARD_CONFIG_SIZE)
  #define STACK_GUARD_BASE    (((uint32_t)((void *)(STACK_BASE)) + STACK_GUARD_SIZE - 1) & ~(STACK_GUARD_SIZE - 1))
  #define REAL_STACK_SIZE     (STACK_SIZE - STACK_GUARD_SIZE)
#else
  #define REAL_STACK_SIZE     STACK_SIZE
#endif

ret_code_t nrf_stack_guard_init();

#if NRF_STACK_GUARD_ENABLED
  #define NRF_STACK_GUARD_INIT() nrf_stack_guard_init()
#else
  #define NRF_STACK_GUARD_INIT() NRF_SUCCESS
#endif

//{{{
#ifdef __cplusplus
}
#endif
//}}}
