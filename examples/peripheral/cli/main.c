//{{{
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "app_timer.h"
#include "fds.h"
#include "app_error.h"
#include "app_util.h"

#include "nrf_cli.h"
#include "nrf_cli_types.h"

#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_flash.h"
#include "nrf_fstorage_nvmc.h"

#include "nrf_mpu.h"
#include "nrf_stack_guard.h"

#include "nrf_cli_uart.h"
//}}}

/* If enabled then CYCCNT (high resolution) timestamp is used for the logger. */
#define USE_CYCCNT_TIMESTAMP_FOR_LOG 0

#if NRF_LOG_BACKEND_FLASHLOG_ENABLED
  NRF_LOG_BACKEND_FLASHLOG_DEF(m_flash_log_backend);
#endif

#if NRF_LOG_BACKEND_CRASHLOG_ENABLED
  NRF_LOG_BACKEND_CRASHLOG_DEF(m_crash_log_backend);
#endif

/* Counter timer. */
APP_TIMER_DEF(m_timer_0);

/* Declared in demo_cli.c */
extern uint32_t m_counter;
extern bool m_counter_active;

#define CLI_EXAMPLE_LOG_QUEUE_SIZE  (4)

NRF_CLI_UART_DEF(m_cli_uart_transport, 0, 64, 16);
NRF_CLI_DEF(m_cli_uart, "uart_cli:~$ ", &m_cli_uart_transport.transport, '\r', CLI_EXAMPLE_LOG_QUEUE_SIZE);

//{{{
static void timer_handle (void* p_context)
{
    UNUSED_PARAMETER(p_context);

    if (m_counter_active)
    {
        m_counter++;
        NRF_LOG_RAW_INFO("counter = %d\r\n", m_counter);
    }
}
//}}}

//{{{
static void cli_start()
{
    ret_code_t ret;

    ret = nrf_cli_start(&m_cli_uart);
    APP_ERROR_CHECK(ret);
}
//}}}
//{{{
static void cli_init()
{
    ret_code_t ret;

    nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
    uart_config.pseltxd = TX_PIN_NUMBER;
    uart_config.pselrxd = RX_PIN_NUMBER;
    uart_config.hwfc    = NRF_UART_HWFC_DISABLED;
    ret = nrf_cli_init(&m_cli_uart, &uart_config, true, true, NRF_LOG_SEVERITY_INFO);
    APP_ERROR_CHECK(ret);
}

//}}}
//{{{
static void cli_process()
{
    nrf_cli_process(&m_cli_uart);
}
//}}}
//{{{
static void flashlog_init()
{
    ret_code_t ret;
    int32_t backend_id;

    ret = nrf_log_backend_flash_init(&nrf_fstorage_nvmc);
    APP_ERROR_CHECK(ret);

#if NRF_LOG_BACKEND_FLASHLOG_ENABLED
    backend_id = nrf_log_backend_add(&m_flash_log_backend, NRF_LOG_SEVERITY_WARNING);
    APP_ERROR_CHECK_BOOL(backend_id >= 0);
    nrf_log_backend_enable(&m_flash_log_backend);
#endif

#if NRF_LOG_BACKEND_CRASHLOG_ENABLED
    backend_id = nrf_log_backend_add(&m_crash_log_backend, NRF_LOG_SEVERITY_INFO);
    APP_ERROR_CHECK_BOOL(backend_id >= 0);
    nrf_log_backend_enable(&m_crash_log_backend);
#endif
}
//}}}

//{{{
static inline void stack_guard_init()
{
    APP_ERROR_CHECK(nrf_mpu_init());
    APP_ERROR_CHECK(nrf_stack_guard_init());
}
//}}}
//{{{
uint32_t cyccnt_get()
{
    return DWT->CYCCNT;
}
//}}}

//{{{
int main() {

  ret_code_t ret;
  if (USE_CYCCNT_TIMESTAMP_FOR_LOG) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    APP_ERROR_CHECK (NRF_LOG_INIT (cyccnt_get, 64000000));
    }
  else {
    APP_ERROR_CHECK (NRF_LOG_INIT (app_timer_cnt_get));
    }

  ret = nrf_drv_clock_init();
  APP_ERROR_CHECK (ret);
  nrf_drv_clock_lfclk_request (NULL);

  ret = app_timer_init();
  APP_ERROR_CHECK(ret);

  ret = app_timer_create (&m_timer_0, APP_TIMER_MODE_REPEATED, timer_handle);
  APP_ERROR_CHECK(ret);

  ret = app_timer_start (m_timer_0, APP_TIMER_TICKS(1000), NULL);
  APP_ERROR_CHECK(ret);

  cli_init();

  //ret = fds_init ();
  APP_ERROR_CHECK (ret);

  UNUSED_RETURN_VALUE (nrf_log_config_load());

  cli_start();
  flashlog_init();
  stack_guard_init();

  NRF_LOG_RAW_INFO("Command Line Interface example started.\r\n");
  NRF_LOG_RAW_INFO("Please press the Tab key to see all available commands.\r\n");

  while (true) {
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
    cli_process();
    }
  }
//}}}
