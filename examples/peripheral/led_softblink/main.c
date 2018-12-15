#include <stdbool.h>
#include <stdint.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "boards.h"
#include "led_softblink.h"
#include "app_error.h"
#include "sdk_errors.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "app_util_platform.h"

static void lfclk_init() {

  uint32_t err_code;
  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK (err_code);
  nrf_drv_clock_lfclk_request (NULL);
  }

int main() {

  lfclk_init();

  // Start APP_TIMER to generate timeouts.
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK (err_code);

  const led_sb_init_params_t led_sb_init_param = LED_SB_INIT_DEFAULT_PARAMS (LEDS_MASK);
  err_code = led_softblink_init (&led_sb_init_param);
  APP_ERROR_CHECK (err_code);

  err_code = led_softblink_start (LEDS_MASK);
  APP_ERROR_CHECK (err_code);

  while (true) {
    __WFE();
    }
  }
