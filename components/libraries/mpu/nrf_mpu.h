#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "sdk_errors.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

/**@brief MPU region handle. */
typedef uint8_t nrf_mpu_region_t;

/**@brief Initialize MPU and driver.
 * @return NRF_SUCCESS on success, otherwise error code.
 */
ret_code_t nrf_mpu_init();

/**@brief Create a new MPU region.
 * @param[out]  p_region    Region handle.
 * @param[in]   address     Region base address.
 * @param[in]   size        Region size.
 * @param[in]   attributes  Region attributes. See ARM Cortex-M MPU documentation.
 * @return NRF_SUCCESS on success, otherwise error code.
 */
ret_code_t nrf_mpu_region_create (nrf_mpu_region_t *p_region, void *address, size_t size, uint32_t attributes);

/**@brief Destroy an MPU region.
 * @param[in]  region    Region handle.
 * @return NRF_SUCCESS on success, otherwise error code.
 */
ret_code_t nrf_mpu_region_destroy (nrf_mpu_region_t region);

//{{{
#ifdef __cplusplus
}
#endif
//}}}
