#include "sdk_common.h"
#if NRF_MODULE_ENABLED(NRF_PWR_MGMT)
  //{{{  includes
  #include "nrf_pwr_mgmt.h"
  #include "nrf.h"
  #include "nrf_mtx.h"
  #include "nrf_power.h"
  #include "app_error.h"
  #include "nrf_assert.h"
  #include "nrf_log_ctrl.h"
  #include "app_util_platform.h"
  //}}}

  #define NRF_LOG_MODULE_NAME pwr_mgmt
  #if NRF_PWR_MGMT_CONFIG_LOG_ENABLED
    #define NRF_LOG_LEVEL       NRF_PWR_MGMT_CONFIG_LOG_LEVEL
    #define NRF_LOG_INFO_COLOR  NRF_PWR_MGMT_CONFIG_INFO_COLOR
    #define NRF_LOG_DEBUG_COLOR NRF_PWR_MGMT_CONFIG_DEBUG_COLOR
  #else
    #define NRF_LOG_LEVEL       0
  #endif

  #include "nrf_log.h"
  NRF_LOG_MODULE_REGISTER();

  #ifdef SOFTDEVICE_PRESENT
    #include "nrf_soc.h"
    #include "nrf_sdh.h"
  #endif

  #if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
    #if (APP_SCHEDULER_ENABLED != 1)
      #error "APP_SCHEDULER is required."
    #endif
    #include "app_scheduler.h"
  #endif

  // Create section "pwr_mgmt_data".
  NRF_SECTION_SET_DEF (pwr_mgmt_data, nrf_pwr_mgmt_shutdown_handler_t, NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT);

  static nrf_pwr_mgmt_evt_t   m_pwr_mgmt_evt;     /**< Event type which will be passed to the shutdown handlers.*/
  static nrf_mtx_t            m_sysoff_mtx;       /**< Module API lock.*/
  static bool                 m_shutdown_started; /**< True if application started the shutdown preparation. */
  static nrf_section_iter_t   m_handlers_iter;    /**< Shutdown handlers iterator. */

  #if (NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED && __FPU_PRESENT)
    #define PWR_MGMT_FPU_SLEEP_PREPARE()  pwr_mgmt_fpu_sleep_prepare()
     //{{{
     __STATIC_INLINE void pwr_mgmt_fpu_sleep_prepare()
     {
        uint32_t original_fpscr;

        CRITICAL_REGION_ENTER();
        original_fpscr = __get_FPSCR();
        /*
         * Clear FPU exceptions.
         * Without this step, the FPU interrupt is marked as pending,
         * preventing system from sleeping. Exceptions cleared:
         * - IOC - Invalid Operation cumulative exception bit.
         * - DZC - Division by Zero cumulative exception bit.
         * - OFC - Overflow cumulative exception bit.
         * - UFC - Underflow cumulative exception bit.
         * - IXC - Inexact cumulative exception bit.
         * - IDC - Input Denormal cumulative exception bit.
         */
        __set_FPSCR(original_fpscr & ~0x9Fu);
        __DMB();
        NVIC_ClearPendingIRQ(FPU_IRQn);
        CRITICAL_REGION_EXIT();

        /*
         * The last chance to indicate an error in FPU to the user
         * as the FPSCR is now cleared
         *
         * This assert is related to previous FPU operations
         * and not power management.
         *
         * Critical FPU exceptions signaled:
         * - IOC - Invalid Operation cumulative exception bit.
         * - DZC - Division by Zero cumulative exception bit.
         * - OFC - Overflow cumulative exception bit.
         */
        ASSERT((original_fpscr & 0x7) == 0);
     }
     //}}}
  #else
    #define PWR_MGMT_FPU_SLEEP_PREPARE()
  #endif

  #if NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED
    #undef  PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED
    #define PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED

    #include "nrf_gpio.h"
    #define PWR_MGMT_DEBUG_PINS_INIT()   pwr_mgmt_debug_pins_init()
    #define PWR_MGMT_DEBUG_PIN_CLEAR()   nrf_gpio_pin_clear(NRF_PWR_MGMT_SLEEP_DEBUG_PIN)
    #define PWR_MGMT_DEBUG_PIN_SET()     nrf_gpio_pin_set(NRF_PWR_MGMT_SLEEP_DEBUG_PIN)
    //{{{
    __STATIC_INLINE void pwr_mgmt_debug_pins_init()
    {
        nrf_gpio_pin_clear(NRF_PWR_MGMT_SLEEP_DEBUG_PIN);
        nrf_gpio_cfg_output(NRF_PWR_MGMT_SLEEP_DEBUG_PIN);
    }
    //}}}
  #else
    #define PWR_MGMT_DEBUG_PIN_CLEAR()
    #define PWR_MGMT_DEBUG_PIN_SET()
    #define PWR_MGMT_DEBUG_PINS_INIT()
  #endif

  #if NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED
    #undef  PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED
    #define PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED

    #undef  PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_TIMER_REQUIRED
    #include "app_timer.h"

    #define PWR_MGMT_CPU_USAGE_MONITOR_INIT()    pwr_mgmt_cpu_usage_monitor_init()
    #define PWR_MGMT_CPU_USAGE_MONITOR_UPDATE()  pwr_mgmt_cpu_usage_monitor_update()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SUMMARY() NRF_LOG_INFO("Maximum CPU usage: %u%%", m_max_cpu_usage)

    //{{{
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_ENTER()  \
        {                                               \
            uint32_t sleep_start = app_timer_cnt_get()
    //}}}
    //{{{
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_EXIT()                 \
            uint32_t sleep_end = app_timer_cnt_get();                 \
            uint32_t sleep_duration;                                  \
            sleep_duration = app_timer_cnt_diff_compute(sleep_end,    \
                                                       sleep_start);  \
            m_ticks_sleeping += sleep_duration;                       \
        }
    //}}}

    static uint32_t m_ticks_sleeping;    /**< Number of ticks spent in sleep mode (__WFE()). */
    static uint32_t m_ticks_last;        /**< Number of ticks from the last CPU usage computation. */
    static uint8_t  m_max_cpu_usage;     /**< Maximum observed CPU usage (0 - 100%). */
    //{{{
    __STATIC_INLINE void pwr_mgmt_cpu_usage_monitor_init()
    {
        m_ticks_sleeping    = 0;
        m_ticks_last        = 0;
        m_max_cpu_usage     = 0;
    }
    //}}}
    //{{{
    __STATIC_INLINE void pwr_mgmt_cpu_usage_monitor_update()
    {
        uint32_t delta;
        uint32_t ticks;
        uint8_t  cpu_usage;

        ticks = app_timer_cnt_get();
        delta = app_timer_cnt_diff_compute(ticks, m_ticks_last);
        cpu_usage = 100 * (delta - m_ticks_sleeping) / delta;

        NRF_LOG_INFO("CPU Usage: %u%%", cpu_usage);
        if (m_max_cpu_usage < cpu_usage)
        {
            m_max_cpu_usage = cpu_usage;
        }

        m_ticks_last        = ticks;
        m_ticks_sleeping    = 0;
    }
    //}}}
  #else
    #define PWR_MGMT_CPU_USAGE_MONITOR_INIT()
    #define PWR_MGMT_CPU_USAGE_MONITOR_UPDATE()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SUMMARY()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_ENTER()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_EXIT()
  #endif

  #if NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
    #undef  PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_STANDBY_TIMEOUT_INIT()  pwr_mgmt_standby_timeout_clear()
    #define PWR_MGMT_STANDBY_TIMEOUT_CLEAR() pwr_mgmt_standby_timeout_clear()
    #define PWR_MGMT_STANDBY_TIMEOUT_CHECK() pwr_mgmt_standby_timeout_check()
    static uint16_t m_standby_counter;     /**< Number of seconds from the last activity @ref pwr_mgmt_feed). */
    //{{{
    __STATIC_INLINE void pwr_mgmt_standby_timeout_clear()
    {
        m_standby_counter = 0;
    }
    //}}}
    //{{{

    __STATIC_INLINE void pwr_mgmt_standby_timeout_check()
    {
        if (m_standby_counter < NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S)
        {
            m_standby_counter++;
        }
        else if (m_shutdown_started == false)
        {
            nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
        }
    }
    //}}}
  #else
    #define PWR_MGMT_STANDBY_TIMEOUT_INIT()
    #define PWR_MGMT_STANDBY_TIMEOUT_CLEAR()
    #define PWR_MGMT_STANDBY_TIMEOUT_CHECK()
  #endif

  #if NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY
    #undef  PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_AUTO_SHUTDOWN_RETRY() pwr_mgmt_auto_shutdown_retry()
    //{{{
    __STATIC_INLINE void pwr_mgmt_auto_shutdown_retry()
    {
        if (m_shutdown_started)
        {
            // Try to continue the shutdown procedure.
            nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_CONTINUE);
        }
    }
    //}}}
  #else
    #define PWR_MGMT_AUTO_SHUTDOWN_RETRY()
  #endif

  #ifdef PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED
    #define PWR_MGMT_SLEEP_INIT()           pwr_mgmt_sleep_init()
    #define PWR_MGMT_SLEEP_LOCK_ACQUIRE()   CRITICAL_REGION_ENTER()
    #define PWR_MGMT_SLEEP_LOCK_RELEASE()   CRITICAL_REGION_EXIT()
    //{{{
    __STATIC_INLINE void pwr_mgmt_sleep_init()
    {
    #ifdef SOFTDEVICE_PRESENT
        ASSERT(current_int_priority_get() >= APP_IRQ_PRIORITY_LOW);
    #endif
        SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
    }
    //}}}
  #else
    #define PWR_MGMT_SLEEP_INIT()
    #define PWR_MGMT_SLEEP_LOCK_ACQUIRE()
    #define PWR_MGMT_SLEEP_LOCK_RELEASE()
  #endif

  #ifdef PWR_MGMT_TIMER_REQUIRED
    #include "app_timer.h"
    #define PWR_MGMT_TIMER_CREATE()  pwr_mgmt_timer_create()
    APP_TIMER_DEF(m_pwr_mgmt_timer);    /**< Timer used by this module. */
    //{{{
    /**@brief Handle events from m_pwr_mgmt_timer.
     */
    static void nrf_pwr_mgmt_timeout_handler(void * p_context)
    {
        PWR_MGMT_CPU_USAGE_MONITOR_UPDATE();
        PWR_MGMT_AUTO_SHUTDOWN_RETRY();
        PWR_MGMT_STANDBY_TIMEOUT_CHECK();
    }
    //}}}
    //{{{
    __STATIC_INLINE ret_code_t pwr_mgmt_timer_create(void)
    {
        ret_code_t ret_code = app_timer_create(&m_pwr_mgmt_timer,
                                               APP_TIMER_MODE_REPEATED,
                                               nrf_pwr_mgmt_timeout_handler);
        if (ret_code != NRF_SUCCESS)
        {
            return ret_code;
        }

        return app_timer_start(m_pwr_mgmt_timer, APP_TIMER_TICKS(1000), NULL);
    }
    //}}}
  #else
    #define PWR_MGMT_TIMER_CREATE() NRF_SUCCESS
  #endif

  //{{{
  ret_code_t nrf_pwr_mgmt_init() {

    NRF_LOG_INFO ("Init");

    m_shutdown_started = false;
    nrf_mtx_init (&m_sysoff_mtx);
    nrf_section_iter_init (&m_handlers_iter, &pwr_mgmt_data);

    PWR_MGMT_SLEEP_INIT();
    PWR_MGMT_DEBUG_PINS_INIT();
    PWR_MGMT_STANDBY_TIMEOUT_INIT();
    PWR_MGMT_CPU_USAGE_MONITOR_INIT();

    return PWR_MGMT_TIMER_CREATE();
    }
  //}}}
  //{{{
  void nrf_pwr_mgmt_run() {

    PWR_MGMT_FPU_SLEEP_PREPARE();
    PWR_MGMT_SLEEP_LOCK_ACQUIRE();
    PWR_MGMT_CPU_USAGE_MONITOR_SECTION_ENTER();
    PWR_MGMT_DEBUG_PIN_SET();

    // Wait for an event.
  #ifdef SOFTDEVICE_PRESENT
    if (nrf_sdh_is_enabled()) {
      ret_code_t ret_code = sd_app_evt_wait();
      ASSERT((ret_code == NRF_SUCCESS) || (ret_code == NRF_ERROR_SOFTDEVICE_NOT_ENABLED));
      UNUSED_VARIABLE(ret_code);
      }
    else
  #endif // SOFTDEVICE_PRESENT
      {
      // Wait for an event.
      __WFE();
      // Clear the internal event register.
      __SEV();
      __WFE();
      }

    PWR_MGMT_DEBUG_PIN_CLEAR();
    PWR_MGMT_CPU_USAGE_MONITOR_SECTION_EXIT();
    PWR_MGMT_SLEEP_LOCK_RELEASE();
    }
  //}}}
  //{{{
  void nrf_pwr_mgmt_feed() {

    NRF_LOG_DEBUG("Feed");
    // It does not stop started shutdown process.
    PWR_MGMT_STANDBY_TIMEOUT_CLEAR();
    }
  //}}}

  //{{{
  static void shutdown_process() {

    NRF_LOG_INFO("Shutdown started. Type %d", m_pwr_mgmt_evt);
    // Executing all callbacks.
    for (/* m_handlers_iter is initialized in nrf_pwr_mgmt_init(). Thanks to that each handler is called only once.*/;
         nrf_section_iter_get(&m_handlers_iter) != NULL; nrf_section_iter_next(&m_handlers_iter))
      {
      nrf_pwr_mgmt_shutdown_handler_t * p_handler = (nrf_pwr_mgmt_shutdown_handler_t *) nrf_section_iter_get(&m_handlers_iter);
      if ((*p_handler)(m_pwr_mgmt_evt)) {
        NRF_LOG_INFO("SysOff handler 0x%08X => ready", (unsigned int)*p_handler);
        }
      else {
        // One of the modules is not ready.
        NRF_LOG_INFO("SysOff handler 0x%08X => blocking", (unsigned int)*p_handler);
        return;
        }
      }

    PWR_MGMT_CPU_USAGE_MONITOR_SUMMARY();
    NRF_LOG_INFO("Shutdown complete.");
    NRF_LOG_FINAL_FLUSH();

    if ((m_pwr_mgmt_evt == NRF_PWR_MGMT_EVT_PREPARE_RESET) || (m_pwr_mgmt_evt == NRF_PWR_MGMT_EVT_PREPARE_DFU))
      NVIC_SystemReset();
    else {
      // Enter System OFF.
      #ifdef SOFTDEVICE_PRESENT
        if (nrf_sdh_is_enabled()) {
          ret_code_t ret_code = sd_power_system_off();
          ASSERT((ret_code == NRF_SUCCESS) || (ret_code == NRF_ERROR_SOFTDEVICE_NOT_ENABLED));
          UNUSED_VARIABLE(ret_code);
          }
      #endif // SOFTDEVICE_PRESENT
      nrf_power_system_off();
      }
    }
  //}}}
  #if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
    //{{{
    static void scheduler_shutdown_handler (void * p_event_data, uint16_t event_size) {

      UNUSED_PARAMETER(p_event_data);
      UNUSED_PARAMETER(event_size);
      shutdown_process();
      }
    //}}}
  #endif

  //{{{
  void nrf_pwr_mgmt_shutdown (nrf_pwr_mgmt_shutdown_t shutdown_type) {

    // Check if shutdown procedure is not started.
    if (!nrf_mtx_trylock(&m_sysoff_mtx))
      return;

    if (shutdown_type != NRF_PWR_MGMT_SHUTDOWN_CONTINUE) {
      if (m_shutdown_started) {
        nrf_mtx_unlock(&m_sysoff_mtx);
        return;
        }
      else {
        m_pwr_mgmt_evt      = (nrf_pwr_mgmt_evt_t)shutdown_type;
        m_shutdown_started  = true;
        }
      }

    ASSERT (m_shutdown_started);
    NRF_LOG_INFO ("Shutdown request %d", shutdown_type);

    #if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
      ret_code_t ret_code = app_sched_event_put (NULL, 0, scheduler_shutdown_handler);
      APP_ERROR_CHECK (ret_code);
    #else
      shutdown_process();
    #endif // NRF_PWR_MGMT_CONFIG_USE_SCHEDULER

    nrf_mtx_unlock (&m_sysoff_mtx);
    }
  //}}}

#endif
