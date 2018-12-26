#include <stdint.h>
#include <stdbool.h>
#include "boards.h"

#if defined (BOARDS_WITH_USB_DFU_TRIGGER) && defined (BOARD_PCA10059)
  #include "nrf_dfu_trigger_usb.h"
#endif

#if LEDS_NUMBER > 0
  static const uint8_t m_board_led_list[LEDS_NUMBER] = LEDS_LIST;
  //{{{
  bool bsp_board_led_state_get (uint32_t led_idx) {

    ASSERT (led_idx < LEDS_NUMBER);
    bool pin_set = nrf_gpio_pin_out_read (m_board_led_list[led_idx]) ? true : false;
    return (pin_set == (LEDS_ACTIVE_STATE ? true : false));
    }
  //}}}

  //{{{
  void bsp_board_led_on (uint32_t led_idx) {

    ASSERT (led_idx < LEDS_NUMBER);
    nrf_gpio_pin_write (m_board_led_list[led_idx], LEDS_ACTIVE_STATE ? 1 : 0);
    }
  //}}}
  //{{{
  void bsp_board_led_off (uint32_t led_idx) {

    ASSERT (led_idx < LEDS_NUMBER);
    nrf_gpio_pin_write(m_board_led_list[led_idx], LEDS_ACTIVE_STATE ? 0 : 1);
    }
  //}}}
  //{{{
  void bsp_board_leds_off() {

    for (uint32_t i = 0; i < LEDS_NUMBER; ++i)
      bsp_board_led_off(i);
    }
  //}}}
  //{{{
  void bsp_board_leds_on() {

    for (uint32_t i = 0; i < LEDS_NUMBER; ++i)
      bsp_board_led_on(i);
    }
  //}}}
  //{{{
  void bsp_board_led_invert (uint32_t led_idx) {

    ASSERT (led_idx < LEDS_NUMBER);
    nrf_gpio_pin_toggle (m_board_led_list[led_idx]);
    }
  //}}}
  //{{{
  uint32_t bsp_board_led_idx_to_pin (uint32_t led_idx) {

    ASSERT(led_idx < LEDS_NUMBER);
    return m_board_led_list[led_idx];
    }
  //}}}
  //{{{
  uint32_t bsp_board_pin_to_led_idx (uint32_t pin_number) {

    uint32_t ret = 0xFFFFFFFF;
    for (uint32_t i = 0; i < LEDS_NUMBER; ++i) {
      if (m_board_led_list[i] == pin_number) {
        ret = i;
        break;
        }
      }
    return ret;
    }
  //}}}

  //{{{
  static void bsp_board_leds_init() {

    for (uint32_t i = 0; i < LEDS_NUMBER; ++i)
      nrf_gpio_cfg_output (m_board_led_list[i]);

    bsp_board_leds_off();
    }
  //}}}
#endif

#if BUTTONS_NUMBER > 0
  static const uint8_t m_board_btn_list[BUTTONS_NUMBER] = BUTTONS_LIST;
  //{{{
  bool bsp_board_button_state_get (uint32_t button_idx) {

    ASSERT(button_idx < BUTTONS_NUMBER);
    bool pin_set = nrf_gpio_pin_read(m_board_btn_list[button_idx]) ? true : false;
    return (pin_set == (BUTTONS_ACTIVE_STATE ? true : false));
    }
  //}}}

  //{{{
  uint32_t bsp_board_pin_to_button_idx (uint32_t pin_number) {

    uint32_t ret = 0xFFFFFFFF;
    for (uint32_t i = 0; i < BUTTONS_NUMBER; ++i) {
      if (m_board_btn_list[i] == pin_number) {
        ret = i;
        break;
        }
      }

    return ret;
    }
  //}}}
  //{{{
  uint32_t bsp_board_button_idx_to_pin (uint32_t button_idx) {

    ASSERT(button_idx < BUTTONS_NUMBER);
    return m_board_btn_list[button_idx];
    }
  //}}}

  //{{{
  static void bsp_board_buttons_init() {

    for (uint32_t i = 0; i < BUTTONS_NUMBER; ++i)
      nrf_gpio_cfg_input(m_board_btn_list[i], BUTTON_PULL);
    }
  //}}}
#endif

//{{{
void bsp_board_init (uint32_t init_flags) {

  #if defined (BOARDS_WITH_USB_DFU_TRIGGER) && defined (BOARD_PCA10059)
    nrf_dfu_trigger_usb_init();
  #endif

  #if defined(BOARD_PCA20020)
    // enable thingy52 VDD_POWER
    nrf_gpio_cfg_output (VDD_PWR_CTRL);
    nrf_gpio_pin_write (VDD_PWR_CTRL, 1);
  #elif defined(BOARD_PCA10059)
    // nRF52 USB Dongle powered from USB (high voltage mode) GPIO out volt 1.8 V by default
    // 3.0v to turn on green and blue LEDs by config UICR register
    if (NRF_POWER->MAINREGSTATUS &
       (POWER_MAINREGSTATUS_MAINREGSTATUS_High << POWER_MAINREGSTATUS_MAINREGSTATUS_Pos)) {
      // config UICR_REGOUT0 register if set to default
      if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) == (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos)) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
          }

        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                            (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
          }

        // System reset is needed to update UICR registers.
        NVIC_SystemReset();
        }
      }
  #endif

  #if LEDS_NUMBER > 0
    if (init_flags & BSP_INIT_LEDS)
      bsp_board_leds_init();
  #endif

  #if BUTTONS_NUMBER > 0
    if (init_flags & BSP_INIT_BUTTONS)
      bsp_board_buttons_init();
  #endif
  }
//}}}
