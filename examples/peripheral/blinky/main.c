// main.c
#include <stdbool.h>
#include <stdint.h>

#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "boards.h"
#if defined (BOARD_PCA20020)
  #include "sx1509b.h"
#endif


//{{{
static void log_init() {

  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK (err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  }
//}}}

int main() {

  log_init();
  bsp_board_init (BSP_INIT_LEDS);

  while (true) {
    for (int i = 0; i < LEDS_NUMBER; i++) {
      bsp_board_led_invert (i);
      nrf_delay_ms (200);
      }
    }
  }
