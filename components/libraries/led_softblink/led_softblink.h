#pragma once
//{{{  includes
#include <stdbool.h>
#include <stdint.h>
#include "sdk_errors.h"
#include "nrf_gpio.h"
//}}}
//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

typedef struct {
  bool      active_high;        /**< Activate negative polarity. */
  uint8_t   duty_cycle_max;     /**< Maximum impulse width. */
  uint8_t   duty_cycle_min;     /**< Minimum impulse width. */
  uint8_t   duty_cycle_step;    /**< Cycle step. */
  uint32_t  off_time_ticks;     /**< Ticks to stay in low impulse state. */
  uint32_t  on_time_ticks;      /**< Ticks to stay in high impulse state. */
  uint32_t  leds_pin_bm;        /**< Mask of used LEDs. */
  NRF_GPIO_Type * p_leds_port;  /**< Port of used LEDs mask. */
  } led_sb_init_params_t;

#define LED_SB_INIT_DEFAULT_PARAMS(mask) { \
  .active_high        = true,              \
  .duty_cycle_max     = 255,               \
  .duty_cycle_min     = 0,                 \
  .duty_cycle_step    = 5,                 \
  .off_time_ticks     = 20000,             \
  .on_time_ticks      = 20000,             \
  .leds_pin_bm        = mask,        \
  .p_leds_port        = NRF_GPIO           \
  }

ret_code_t led_softblink_start (uint32_t leds_pin_bit_mask);
ret_code_t led_softblink_stop();

void led_softblink_off_time_set (uint32_t off_time_ticks);
void led_softblink_on_time_set (uint32_t on_time_ticks);

ret_code_t led_softblink_init (led_sb_init_params_t const * p_init_params);
ret_code_t led_softblink_uninit();

//{{{
#ifdef __cplusplus
}
#endif
//}}}
