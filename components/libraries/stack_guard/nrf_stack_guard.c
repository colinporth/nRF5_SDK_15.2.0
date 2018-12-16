#include <stdint.h>
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_strerror.h"
#include "nrf_mpu.h"
#include "nrf_stack_guard.h"

#define NRF_LOG_MODULE_NAME stack_guard

#if NRF_STACK_GUARD_CONFIG_LOG_ENABLED
  #define NRF_LOG_LEVEL       NRF_STACK_GUARD_CONFIG_LOG_LEVEL
  #define NRF_LOG_INFO_COLOR  NRF_STACK_GUARD_CONFIG_INFO_COLOR
  #define NRF_LOG_DEBUG_COLOR NRF_STACK_GUARD_CONFIG_DEBUG_COLOR
  #endif

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#if NRF_STACK_GUARD_ENABLED
  STATIC_ASSERT(STACK_GUARD_SIZE >= 32);

  ret_code_t nrf_stack_guard_init(void)
  {
      nrf_mpu_region_t region;
      uint32_t attributes;
      ret_code_t status;

      ASSERT((STACK_GUARD_BASE + STACK_GUARD_SIZE) < (uint32_t)((void *)(STACK_TOP)));

      attributes = (0x05 << MPU_RASR_TEX_Pos) | (1 << MPU_RASR_B_Pos) |   /* Normal memory, WBWA/WBWA */
                   (0x07 << MPU_RASR_AP_Pos) | (1 << MPU_RASR_XN_Pos);    /* Access: RO/RO, XN */

      status = nrf_mpu_region_create(&region, (void*)(STACK_GUARD_BASE), STACK_GUARD_SIZE, attributes);
      if (status == NRF_SUCCESS)
      {
          NRF_LOG_INFO("Stack Guard (%u bytes): 0x%08X-0x%08X (total stack size: %u bytes, usable stack area: %u bytes)",
                       STACK_GUARD_SIZE, STACK_GUARD_BASE, STACK_GUARD_BASE + STACK_GUARD_SIZE - 1,
                       STACK_SIZE, REAL_STACK_SIZE);
      }
      else
      {
          NRF_LOG_ERROR("Cannot create stack guard page! Error: %u [%s]", status, (uint32_t)nrf_strerror_get(status));
      }

      return status;
  }

#endif /* NRF_STACK_GUARD_ENABLED */
