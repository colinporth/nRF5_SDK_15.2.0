#pragma once
#include <stdint.h>
#include "sdk_errors.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

//{{{
/**@brief Function for initializing the NFC Button Module.
 *
 * Before calling this function, the BSP module must be initialized with buttons. The chosen
 * button is used to generate @ref BSP_EVENT_SLEEP events.
 *
 * @param[in]   sleep_button    Button ID used to generate @ref BSP_EVENT_SLEEP event.
 *
 * @retval      NRF_SUCCESS     If initialization was successful. Otherwise, a propagated error
 *                              code is returned.
 */
ret_code_t bsp_nfc_btn_init(uint32_t sleep_button);
//}}}
//{{{
/**@brief Function for deinitializing the NFC Button Module.
 *
 * Before calling this function, the BSP module must be initialized with buttons. The chosen
 * button is used to generate default @ref BSP_EVENT_DEFAULT events.
 *
 * @param[in]   sleep_button    Button ID used to restore default event generation.
 *
 * @retval      NRF_SUCCESS     If initialization was successful. Otherwise, a propagated error
 *                              code is returned.
 */
ret_code_t bsp_nfc_btn_deinit(uint32_t sleep_button);
//}}}
//{{{
/**@brief Function for setting up NFCT peripheral as wake-up source.
 *
 * This function must be called before going into System OFF mode.
 *
 * @note This function is only applicable if NFCT is used exclusively for wakeup.
 *       If NFCT is used for a different purpose, this function cannot be used.
 *
 * @retval      NRF_SUCCESS     If NFCT peripheral was prepared successfully. Otherwise,
 *                              a propagated error code is returned.
 */
ret_code_t bsp_nfc_sleep_mode_prepare(void);
//}}}

//{{{
#ifdef __cplusplus
}
#endif
//}}}
