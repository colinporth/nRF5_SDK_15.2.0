#pragma once
//{{{
#include <nrfx.h>
#include "app_timer.h"
#include "sdk_errors.h"
//}}}
//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

typedef enum {
  LOW_POWER_PWM_EVENT_PERIOD = 0,
  LOW_POWER_PWM_EVENT_DUTY_CYCLE
  } low_power_pwm_evt_type_t;

typedef void (*low_power_pwm_timeout_user)(void * p_context, low_power_pwm_evt_type_t evt_type);

typedef struct {
  bool                    active_high; /**< Activate negative polarity. */
  uint8_t                 period;      /**< Width of the low_power_pwm period. */
  NRF_GPIO_Type *         p_port;      /**< Port used to work on selected mask. */
  uint32_t                bit_mask;    /**< Pins to be initialized. */
  app_timer_id_t const *  p_timer_id;  /**< Pointer to the timer ID of low_power_pwm. */
  } low_power_pwm_config_t;

#define LOW_POWER_PWM_CONFIG_ACTIVE_HIGH    false
#define LOW_POWER_PWM_CONFIG_PERIOD         UINT8_MAX
#define LOW_POWER_PWM_CONFIG_PORT           NRF_GPIO
#define LOW_POWER_PWM_CONFIG_BIT_MASK(mask) (mask)

#define LOW_POWER_PWM_DEFAULT_CONFIG(mask) {         \
  .active_high = LOW_POWER_PWM_CONFIG_ACTIVE_HIGH ,  \
  .period      = LOW_POWER_PWM_CONFIG_PERIOD   ,     \
  .p_port      = LOW_POWER_PWM_CONFIG_PORT,          \
  .bit_mask    = LOW_POWER_PWM_CONFIG_BIT_MASK(mask) \
  }

struct low_power_pwm_s {
  bool                        active_high;        /**< Activate negative polarity. */
  bool                        pin_is_on;          /**< Indicates the current state of the pin. */
  uint8_t                     period;             /**< Width of the low_power_pwm period. */
  uint8_t                     duty_cycle;         /**< Width of high pulse. */
  nrfx_drv_state_t            pwm_state;          /**< Indicates the current state of the PWM instance. */
  uint32_t                    bit_mask;           /**< Pins to be initialized. */
  uint32_t                    bit_mask_toggle;    /**< Pins to be toggled. */
  uint32_t                    timeout_ticks;      /**< Value to start the next app_timer. */
  low_power_pwm_evt_type_t    evt_type;           /**< Slope that triggered time-out. */
  app_timer_timeout_handler_t handler;            /**< User handler to be called in the time-out handler. */
  app_timer_id_t const *      p_timer_id;         /**< Pointer to the timer ID of low_power_pwm. */
  NRF_GPIO_Type *             p_port;             /**< Port used with pin bit mask. */
  };

typedef struct low_power_pwm_s low_power_pwm_t;
//{{{
/**
 * @brief   Function for initializing a low-power PWM instance.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be started.
 * @param[in] p_pwm_config              Pointer to the configuration structure.
 * @param[in] handler                   User function to be called in case of time-out.
 *
 * @return Values returned by @ref app_timer_create.
 */
ret_code_t low_power_pwm_init (low_power_pwm_t* p_pwm_instance,
                              low_power_pwm_config_t const * p_pwm_config,
                              app_timer_timeout_handler_t handler);

//}}}
//{{{
/**
 * @brief   Function for starting a low-power PWM instance.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be started.
 * @param[in] pins_bit_mask             Bit mask of pins to be started.
 *
 * @return Values returned by @ref app_timer_start.
 */
ret_code_t low_power_pwm_start (low_power_pwm_t* p_pwm_instance,
                               uint32_t          pins_bit_mask);

//}}}
//{{{
/**
 * @brief   Function for stopping a low-power PWM instance.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be stopped.
 *
 * @return Values returned by @ref app_timer_stop.
 */
ret_code_t low_power_pwm_stop (low_power_pwm_t* p_pwm_instance);
//}}}
//{{{
/**
 * @brief   Function for setting a new high pulse width for a given instance.
 *
 * This function can be called from the timer handler.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be changed.
 * @param[in] duty_cycle                New high pulse width. 0 means that the pin is always off. 255 means that it is always on.
 *
 * @retval NRF_SUCCESS                  If the function completed successfully.
 * @retval NRF_ERROR_INVALID_PARAM      If the function returned an error because of invalid parameters.
 */
ret_code_t low_power_pwm_duty_set (low_power_pwm_t* p_pwm_instance, uint8_t duty_cycle);
//}}}

//{{{
#ifdef __cplusplus
}
#endif
//}}}
