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
  bool            active_high;     /**< Activate negative polarity. */
  uint8_t         duty_cycle_max;  /**< Maximum impulse width. */
  uint8_t         duty_cycle_min;  /**< Minimum impulse width. */
  uint8_t         duty_cycle_step; /**< Cycle step. */
  uint32_t        off_time_ticks;  /**< Ticks to stay in low impulse state. */
  uint32_t        on_time_ticks;   /**< Ticks to stay in high impulse state. */
  uint32_t        leds_pin_bm;     /**< Mask of used LEDs. */
  NRF_GPIO_Type * p_leds_port;     /**< Port of used LEDs mask. */
  } led_sb_init_params_t;

#define LED_SB_INIT_PARAMS_ACTIVE_HIGH            false
#define LED_SB_INIT_PARAMS_DUTY_CYCLE_MAX         220
#define LED_SB_INIT_PARAMS_DUTY_CYCLE_MIN         0
#define LED_SB_INIT_PARAMS_DUTY_CYCLE_STEP        5
#define LED_SB_INIT_PARAMS_OFF_TIME_TICKS         65536
#define LED_SB_INIT_PARAMS_ON_TIME_TICKS          0
#define LED_SB_INIT_PARAMS_LEDS_PIN_BM(mask)      (mask)
#define LED_SB_INIT_PARAMS_LEDS_PORT              NRF_GPIO

#define LED_SB_INIT_DEFAULT_PARAMS(mask) {                         \
  .active_high        = LED_SB_INIT_PARAMS_ACTIVE_HIGH,           \
  .duty_cycle_max     = LED_SB_INIT_PARAMS_DUTY_CYCLE_MAX,        \
  .duty_cycle_min     = LED_SB_INIT_PARAMS_DUTY_CYCLE_MIN,        \
  .duty_cycle_step    = LED_SB_INIT_PARAMS_DUTY_CYCLE_STEP,       \
  .off_time_ticks     = LED_SB_INIT_PARAMS_OFF_TIME_TICKS,        \
  .on_time_ticks      = LED_SB_INIT_PARAMS_ON_TIME_TICKS,         \
  .leds_pin_bm        = LED_SB_INIT_PARAMS_LEDS_PIN_BM(mask),     \
  .p_leds_port        = LED_SB_INIT_PARAMS_LEDS_PORT              \
  }

ret_code_t led_softblink_init (led_sb_init_params_t const * p_init_params);
ret_code_t led_softblink_start (uint32_t leds_pin_bit_mask);
ret_code_t led_softblink_stop();

void led_softblink_off_time_set (uint32_t off_time_ticks);
void led_softblink_on_time_set (uint32_t on_time_ticks);

ret_code_t led_softblink_uninit();

//{{{
#ifdef __cplusplus
}
#endif
//}}}
