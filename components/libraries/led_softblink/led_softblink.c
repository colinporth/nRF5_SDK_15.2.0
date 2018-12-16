#include "sdk_common.h"

#if NRF_MODULE_ENABLED(LED_SOFTBLINK)
  //{{{  includes
  #include <string.h>
  #include "led_softblink.h"
  #include "nrf_gpio.h"
  #include "app_timer.h"
  #include "nrf_assert.h"
  #include "low_power_pwm.h"
  //}}}

  #define PWM_PERIOD UINT8_MAX

  typedef struct {
    bool                    leds_is_on;     /**< Flag for indicating if LEDs are on. */
    bool                    is_counting_up; /**< Flag for indicating if counter is incrementing or decrementing. */
    nrfx_drv_state_t        led_sb_state;   /**< Indicates current state of instance. */
    uint16_t                duty_cycle;     /**< Current pulse width. */
    uint32_t                bit_mask;       /**< Mask of used pins. */
    led_sb_init_params_t    params;         /**< Structure holding initialization parameters. */
    low_power_pwm_config_t  pwm_config;     /**< Structure holding parameters for initializing low level layer. */
    low_power_pwm_t         pwm_instance;   /**< Structure holding low-power PWM instance parameters. */
    } led_sb_context_t;

  APP_TIMER_DEF(m_led_softblink_timer);

  static led_sb_context_t m_led_sb = { 0 };

  //{{{
  /**@brief Timer event handler for softblink.
   *
   * @param[in] p_context            General purpose pointer. Will be passed to the time-out handler
   *                                when the timer expires.
   */
  static void led_softblink_on_timeout (void* p_context)
  {
      static int32_t pause_ticks;
      ASSERT(m_led_sb.led_sb_state != NRFX_DRV_STATE_UNINITIALIZED);
      ret_code_t err_code;

      if (pause_ticks <= 0)
      {
          if (m_led_sb.is_counting_up)
          {
              if (m_led_sb.duty_cycle >= (m_led_sb.params.duty_cycle_max - m_led_sb.params.duty_cycle_step))
              {
                  // Max PWM duty cycle is reached, start decrementing.
                  m_led_sb.is_counting_up = false;
                  m_led_sb.duty_cycle     = m_led_sb.params.duty_cycle_max;
                  pause_ticks = m_led_sb.params.on_time_ticks ? m_led_sb.params.on_time_ticks + APP_TIMER_MIN_TIMEOUT_TICKS : 0;
              }
              else
              {
                  m_led_sb.duty_cycle += m_led_sb.params.duty_cycle_step;
              }
          }
          else
          {
              if (m_led_sb.duty_cycle <= (m_led_sb.params.duty_cycle_min + m_led_sb.params.duty_cycle_step))
              {
                  // Min PWM duty cycle is reached, start incrementing.
                  m_led_sb.is_counting_up = true;
                  m_led_sb.duty_cycle     = m_led_sb.params.duty_cycle_min;
                  pause_ticks = m_led_sb.params.off_time_ticks ? m_led_sb.params.off_time_ticks + APP_TIMER_MIN_TIMEOUT_TICKS : 0;
              }
              else
              {
                  m_led_sb.duty_cycle -= m_led_sb.params.duty_cycle_step;
              }
          }
      }
      else
      {
          pause_ticks -= PWM_PERIOD;
      }

      err_code = low_power_pwm_duty_set(&m_led_sb.pwm_instance, m_led_sb.duty_cycle);

      APP_ERROR_CHECK(err_code);
    }
  //}}}

  //{{{
  ret_code_t led_softblink_start (uint32_t leds_pin_bit_mask) {

    ASSERT (m_led_sb.led_sb_state == NRFX_DRV_STATE_INITIALIZED);
    return low_power_pwm_start (&m_led_sb.pwm_instance, leds_pin_bit_mask);
    }
  //}}}
  //{{{
  ret_code_t led_softblink_stop() {

    return low_power_pwm_stop (&m_led_sb.pwm_instance);
    }
  //}}}

  //{{{
  void led_softblink_on_time_set (uint32_t on_time_ticks) {

    ASSERT(m_led_sb.led_sb_state != NRFX_DRV_STATE_UNINITIALIZED);
    m_led_sb.params.on_time_ticks = on_time_ticks;
    }
  //}}}
  //{{{
  void led_softblink_off_time_set (uint32_t off_time_ticks) {

    ASSERT(m_led_sb.led_sb_state != NRFX_DRV_STATE_UNINITIALIZED);
    m_led_sb.params.off_time_ticks = off_time_ticks;
    }
  //}}}

  //{{{
  ret_code_t led_softblink_init (led_sb_init_params_t const* p_init_params)
  {
      ret_code_t err_code;

      ASSERT(m_led_sb.led_sb_state == NRFX_DRV_STATE_UNINITIALIZED);
      ASSERT(p_init_params);

      if ( (p_init_params->duty_cycle_max == 0)                               ||
           (p_init_params->duty_cycle_max <= p_init_params->duty_cycle_min)   ||
           (p_init_params->duty_cycle_step == 0)                              ||
           (p_init_params->leds_pin_bm == 0))
      {
          return NRF_ERROR_INVALID_PARAM;
      }



      memset(&m_led_sb, 0, sizeof(led_sb_context_t));
      memcpy(&m_led_sb.params, p_init_params, sizeof(led_sb_init_params_t));

      m_led_sb.is_counting_up = true;
      m_led_sb.duty_cycle     = p_init_params->duty_cycle_min + p_init_params->duty_cycle_step;
      m_led_sb.leds_is_on     = false;
      m_led_sb.bit_mask       = p_init_params->leds_pin_bm;

      m_led_sb.pwm_config.active_high         = m_led_sb.params.active_high;
      m_led_sb.pwm_config.bit_mask            = p_init_params->leds_pin_bm;
      m_led_sb.pwm_config.p_port              = p_init_params->p_leds_port;
      m_led_sb.pwm_config.period              = PWM_PERIOD;
      m_led_sb.pwm_config.p_timer_id          = &m_led_softblink_timer;

      err_code = low_power_pwm_init( &m_led_sb.pwm_instance, &m_led_sb.pwm_config, led_softblink_on_timeout);

      if (err_code == NRF_SUCCESS)
      {
          m_led_sb.led_sb_state = NRFX_DRV_STATE_INITIALIZED;
      }
      else
      {
          return err_code;
      }

      err_code = low_power_pwm_duty_set( &m_led_sb.pwm_instance, p_init_params->duty_cycle_min + p_init_params->duty_cycle_step);

      return err_code;
  }

  //}}}
  //{{{
  ret_code_t led_softblink_uninit() {

    ASSERT(m_led_sb.led_sb_state != NRFX_DRV_STATE_UNINITIALIZED);

    ret_code_t err_code = led_softblink_stop();
    if (err_code == NRF_SUCCESS)
        m_led_sb.led_sb_state = NRFX_DRV_STATE_UNINITIALIZED;
    else
        return err_code;

    memset (&m_led_sb, 0, sizeof(m_led_sb));
    return NRF_SUCCESS;
    }
  //}}}
#endif
