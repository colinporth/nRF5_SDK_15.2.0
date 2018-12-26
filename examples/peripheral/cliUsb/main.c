//{{{  includes
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>

#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "app_timer.h"
#include "fds.h"
#include "app_error.h"
#include "app_util.h"

#include "nrf_cli.h"
#include "nrf_cli_rtt.h"
#include "nrf_cli_types.h"

#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_flash.h"
#include "nrf_fstorage_nvmc.h"

#include "nrf_mpu.h"
#include "nrf_stack_guard.h"

// usb acm includes
#include "nrf_cli_cdc_acm.h"

#include "nrf_drv_usbd.h"

#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"

#include "boards.h"
//}}}

#define USBD_POWER_DETECTION true
#define USE_CYCCNT_TIMESTAMP_FOR_LOG 0

static uint32_t gCounter;
static bool gCounter_active = false;
APP_TIMER_DEF (m_timer_0);
//{{{
static void timerHandle (void* context) {

  if (gCounter_active) {
    gCounter++;
    NRF_LOG_RAW_INFO ("counter = %d\r\n", gCounter);
    }
  }
//}}}

#define CLI_EXAMPLE_MAX_CMD_CNT  20
#define CLI_EXAMPLE_MAX_CMD_LEN  33
static char m_dynamic_cmd_buffer[CLI_EXAMPLE_MAX_CMD_CNT][CLI_EXAMPLE_MAX_CMD_LEN];
static uint8_t m_dynamic_cmd_cnt;

NRF_CLI_CDC_ACM_DEF (mCliCdcAcmTransport);
NRF_CLI_DEF (mCliCdcAcm, "cliUsb:~$ ", &mCliCdcAcmTransport.transport, '\r', 4);
NRF_CLI_RTT_DEF (mCliRttTransport);
NRF_CLI_DEF (mCliRtt, "rtt_cli:~$ ", &mCliRttTransport.transport, '\n', 4);

NRF_LOG_BACKEND_FLASHLOG_DEF (mFlashLogBackend);
NRF_LOG_BACKEND_CRASHLOG_DEF (mCrashLogBackend);

//{{{  led commands
static void cmd_led (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  if (nrf_cli_help_requested (p_cli)) {
    nrf_cli_help_print (p_cli, NULL, 0);
    return;
    }
  int i = strtol (argv[1], NULL, 10);
  nrf_cli_fprintf (p_cli, NRF_CLI_OPTION,  "Control leds %d\r\n", i);
  bsp_board_led_invert (i);
  }

static void cmd_cycle (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  if (nrf_cli_help_requested (p_cli)) {
    nrf_cli_help_print (p_cli, NULL, 0);
    return;
    }
  nrf_cli_fprintf (p_cli, NRF_CLI_OPTION,  "cycle leds\r\n");
  for (int j = 0; j < 8; j++)
    for (int i = 0; i < LEDS_NUMBER; i++) {
      bsp_board_led_invert (i);
      nrf_delay_ms (200);
      }
  }

NRF_CLI_CMD_REGISTER (led, NULL, "control leds", cmd_led);
NRF_CLI_CMD_REGISTER (cycle, NULL, "cycle leds", cmd_cycle);
//}}}
//{{{  nordic command
static void cmd_nordic (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if (nrf_cli_help_requested (p_cli)) {
    nrf_cli_help_print (p_cli, NULL, 0);
    return;
    }

  nrf_cli_fprintf (p_cli, NRF_CLI_OPTION,
    "\r\n"
    "            .co:.                   'xo,          \r\n"
    "         .,collllc,.             'ckOOo::,..      \r\n"
    "      .:ooooollllllll:'.     .;dOOOOOOo:::;;;'.   \r\n"
    "   'okxddoooollllllllllll;'ckOOOOOOOOOo:::;;;,,,' \r\n"
    "   OOOkxdoooolllllllllllllllldxOOOOOOOo:::;;;,,,'.\r\n"
    "   OOOOOOkdoolllllllllllllllllllldxOOOo:::;;;,,,'.\r\n"
    "   OOOOOOOOOkxollllllllllllllllllcccldl:::;;;,,,'.\r\n"
    "   OOOOOOOOOOOOOxdollllllllllllllccccc::::;;;,,,'.\r\n"
    "   OOOOOOOOOOOOOOOOkxdlllllllllllccccc::::;;;,,,'.\r\n"
    "   kOOOOOOOOOOOOOOOOOOOkdolllllllccccc::::;;;,,,'.\r\n"
    "   kOOOOOOOOOOOOOOOOOOOOOOOxdllllccccc::::;;;,,,'.\r\n"
    "   kOOOOOOOOOOOOOOOOOOOOOOOOOOkxolcccc::::;;;,,,'.\r\n"
    "   kOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOkdlc::::;;;,,,'.\r\n"
    "   xOOOOOOOOOOOxdkOOOOOOOOOOOOOOOOOOOOxoc:;;;,,,'.\r\n"
    "   xOOOOOOOOOOOdc::ldkOOOOOOOOOOOOOOOOOOOkdc;,,,''\r\n"
    "   xOOOOOOOOOOOdc::;;,;cdkOOOOOOOOOOOOOOOOOOOxl;''\r\n"
    "   .lkOOOOOOOOOdc::;;,,''..;oOOOOOOOOOOOOOOOOOOOx'\r\n"
    "      .;oOOOOOOdc::;;,.       .:xOOOOOOOOOOOOd;.  \r\n"
    "          .:xOOdc:,.              'ckOOOOkl'      \r\n"
    "             .od'                    'xk,         \r\n"
    "\r\n");

  nrf_cli_fprintf (p_cli,NRF_CLI_NORMAL,
    "                Nordic Semiconductor              \r\n\r\n");
  }


NRF_CLI_CMD_REGISTER (nordic, NULL, "Print Nordic Semiconductor logo.", cmd_nordic);
//}}}
//{{{  counter commands
static void cmd_counter_start (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  if (argc != 1) {
    nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }
  gCounter_active = true;
  }


static void cmd_counter_stop (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  if (argc != 1) {
    nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }
  gCounter_active = false;
  }


static void cmd_counter_reset (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  if (argc != 1) {
    nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }
  gCounter = 0;
  }


static void cmd_counter (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  ASSERT (p_cli);
  ASSERT (p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);
  static const nrf_cli_getopt_option_t opt[] = { NRF_CLI_OPT ("--test", "-t", "dummy option help string" ) };
  if ((argc == 1) || nrf_cli_help_requested (p_cli)) {
    nrf_cli_help_print (p_cli, opt, ARRAY_SIZE (opt));
    return;
    }
  if (argc != 2) {
    nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }
  if (!strcmp (argv[1], "-t") || !strcmp (argv[1], "--test")) {
    nrf_cli_fprintf (p_cli, NRF_CLI_NORMAL, "Dummy test option.\r\n");
    return;
    }

  // subcommands have their own handlers and they are not processed here
  nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\r\n", argv[0], argv[1]);
  }


NRF_CLI_CREATE_STATIC_SUBCMD_SET (m_sub_counter) {
  NRF_CLI_CMD (reset,  NULL, "Reset seconds counter.",  cmd_counter_reset),
  NRF_CLI_CMD (start,  NULL, "Start seconds counter.",  cmd_counter_start),
  NRF_CLI_CMD (stop,   NULL, "Stop seconds counter.",   cmd_counter_stop),
  NRF_CLI_SUBCMD_SET_END
  };

NRF_CLI_CMD_REGISTER (counter, &m_sub_counter, "Display seconds on terminal screen", cmd_counter);
//}}}
//{{{  stack overflow command
#define CLI_EXAMPLE_VALUE_BIGGER_THAN_STACK  (20000u)

// This function cannot be static otherwise it can be inlined. As a result, variable:
//   tab[CLI_EXAMPLE_VALUE_BIGGER_THAN_STACK] will be always created on stack. This will block
//   possiblity to call functions: nrf_cli_help_requested and nrf_cli_help_print within
//   cmd_stack_overflow, because stack guard will be triggered. */
void cli_example_stack_overflow_force() {
  char tab[CLI_EXAMPLE_VALUE_BIGGER_THAN_STACK];
  volatile char* p_tab = tab;
  // This function accesses stack area protected by nrf_stack_guard. As a result
  // MPU (memory protection unit) triggers an exception (hardfault). Hardfault handler will log exception reason
  for (size_t idx = 0; idx < STACK_SIZE; idx++)
    *(p_tab + idx) = (uint8_t)idx;
  }


static void cmd_stack_overflow (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  if (nrf_cli_help_requested (p_cli)) {
    nrf_cli_help_print (p_cli, NULL, 0);
    return;
    }
  cli_example_stack_overflow_force();
  }


NRF_CLI_CMD_REGISTER (stack_overflow, NULL,
                      "Command tests nrf_stack_guard module. Upon command call stack will be "
                      "overflowed and microcontroller shall log proper reset reason. \n\rTo observe "
                      "stack_guard execution, stack shall be set to value lower than 20000 bytes.",
                      cmd_stack_overflow);
//}}}
//{{{  print commands
static void cmd_print_param (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  for (size_t i = 1; i < argc; i++)
    nrf_cli_fprintf (p_cli, NRF_CLI_NORMAL, "argv[%d] = %s\r\n", i, argv[i]);
  }


static void cmd_print_all (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  for (size_t i = 1; i < argc; i++)
    nrf_cli_fprintf (p_cli, NRF_CLI_NORMAL, "%s ", argv[i]);
  nrf_cli_fprintf (p_cli, NRF_CLI_NORMAL, "\r\n");
  }


static void cmd_print (nrf_cli_t const* p_cli, size_t argc, char** argv) {
  ASSERT(p_cli);
  ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

  if ((argc == 1) || nrf_cli_help_requested (p_cli)) {
    nrf_cli_help_print (p_cli, NULL, 0);
    return;
    }

  if (argc != 2) {
    nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\r\n", argv[0], argv[1]);
  }


NRF_CLI_CREATE_STATIC_SUBCMD_SET (m_sub_print) {
  NRF_CLI_CMD (all,   NULL, "Print all entered parameters.", cmd_print_all),
  NRF_CLI_CMD (param, NULL, "Print each parameter in new line.", cmd_print_param),
  NRF_CLI_SUBCMD_SET_END
  };

NRF_CLI_CMD_REGISTER (print, &m_sub_print, "print", cmd_print);
//}}}
//{{{  dynamic commands
//{{{
/* function required by qsort */
static int string_cmp (const void* p_a, const void * p_b) {

  ASSERT(p_a);
  ASSERT(p_b);
  return strcmp((const char *)p_a, (const char *)p_b);
  }
//}}}
//{{{
static void cmd_dynamic (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if ((argc == 1) || nrf_cli_help_requested(p_cli)) {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
    }

  if (argc > 2)
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
  else
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: please specify subcommand\r\n", argv[0]);
  }
//}}}
//{{{
static void cmd_dynamic_add (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if (nrf_cli_help_requested(p_cli)) {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
    }

  if (argc != 2) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  if (m_dynamic_cmd_cnt >= CLI_EXAMPLE_MAX_CMD_CNT) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "command limit reached\r\n");
    return;
    }

  uint8_t idx;
  nrf_cli_cmd_len_t cmd_len = strlen(argv[1]);

  if (cmd_len >= CLI_EXAMPLE_MAX_CMD_LEN) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "too long command\r\n");
    return;
    }

  for (idx = 0; idx < cmd_len; idx++) {
    if (!isalnum((int)(argv[1][idx]))) {
      nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "bad command name - please use only alphanumerical characters\r\n");
      return;
      }
    }

  for (idx = 0; idx < CLI_EXAMPLE_MAX_CMD_CNT; idx++) {
    if (!strcmp(m_dynamic_cmd_buffer[idx], argv[1])) {
      nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "duplicated command\r\n");
      return;
      }
    }

  sprintf (m_dynamic_cmd_buffer[m_dynamic_cmd_cnt++], "%s", argv[1]);
  qsort (m_dynamic_cmd_buffer, m_dynamic_cmd_cnt, sizeof (m_dynamic_cmd_buffer[0]), string_cmp);
  nrf_cli_fprintf (p_cli, NRF_CLI_NORMAL, "command added successfully\r\n");
  }
//}}}
//{{{
static void cmd_dynamic_show (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if (nrf_cli_help_requested(p_cli)) {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
    }

 if (argc != 1) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  if (m_dynamic_cmd_cnt == 0) {
    nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "Please add some commands first.\r\n");
    return;
    }

  nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Dynamic command list:\r\n");
  for (uint8_t i = 0; i < m_dynamic_cmd_cnt; i++)
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "[%3d] %s\r\n", i, m_dynamic_cmd_buffer[i]);
  }
//}}}
//{{{
static void cmd_dynamic_execute (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if (nrf_cli_help_requested (p_cli)) {
    nrf_cli_help_print (p_cli, NULL, 0);
    return;
    }

  if (argc != 2) {
    nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  for (uint8_t idx = 0; idx <  m_dynamic_cmd_cnt; idx++) {
    if (!strcmp (m_dynamic_cmd_buffer[idx], argv[1])) {
      nrf_cli_fprintf (p_cli, NRF_CLI_NORMAL, "dynamic command: %s\r\n", argv[1]);
      return;
      }
    }

  nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: uknown parameter: %s\r\n", argv[0], argv[1]);
  }
//}}}
//{{{
static void cmd_dynamic_remove (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if ((argc == 1) || nrf_cli_help_requested(p_cli)) {
    nrf_cli_help_print (p_cli, NULL, 0);
    return;
    }

  if (argc != 2) {
    nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  for (uint8_t idx = 0; idx <  m_dynamic_cmd_cnt; idx++) {
    if (!strcmp (m_dynamic_cmd_buffer[idx], argv[1])) {
      if (idx == CLI_EXAMPLE_MAX_CMD_CNT - 1)
        m_dynamic_cmd_buffer[idx][0] = '\0';
      else
        memmove (m_dynamic_cmd_buffer[idx], m_dynamic_cmd_buffer[idx + 1],
                 sizeof(m_dynamic_cmd_buffer[idx]) * (m_dynamic_cmd_cnt - idx));
      --m_dynamic_cmd_cnt;
      nrf_cli_fprintf (p_cli, NRF_CLI_NORMAL, "command removed successfully\r\n");
      return;
      }
    }

  nrf_cli_fprintf (p_cli, NRF_CLI_ERROR, "did not find command: %s\r\n", argv[1]);
  }
//}}}
//{{{
static void dynamic_cmd_get (size_t idx, nrf_cli_static_entry_t* p_static) {

  ASSERT(p_static);

  if (idx < m_dynamic_cmd_cnt) {
    /* m_dynamic_cmd_buffer must be sorted alphabetically to ensure correct CLI completion */
    p_static->p_syntax = m_dynamic_cmd_buffer[idx];
    p_static->handler  = NULL;
    p_static->p_subcmd = NULL;
    p_static->p_help = "Show dynamic command name.";
    }
  else
    /* if there are no more dynamic commands available p_syntax must be set to NULL */
    p_static->p_syntax = NULL;
  }
//}}}

NRF_CLI_CREATE_DYNAMIC_CMD (m_sub_dynamic_set, dynamic_cmd_get);

NRF_CLI_CREATE_STATIC_SUBCMD_SET (m_sub_dynamic) {

  NRF_CLI_CMD (add, NULL,
    "Add a new dynamic command.\nExample usage: [ dynamic add test ] will add "
    "a dynamic command 'test'.\nIn this example, command name length is limited to 32 chars. "
    "You can add up to 20 commands. Commands are automatically sorted to ensure correct "
    "CLI completion.",
    cmd_dynamic_add),

  NRF_CLI_CMD (execute, &m_sub_dynamic_set, "Execute a command.", cmd_dynamic_execute),
  NRF_CLI_CMD (remove, &m_sub_dynamic_set, "Remove a command.", cmd_dynamic_remove),
  NRF_CLI_CMD (show, NULL, "Show all added dynamic commands.", cmd_dynamic_show),
  NRF_CLI_SUBCMD_SET_END
  };

NRF_CLI_CMD_REGISTER (dynamic, &m_sub_dynamic, "Demonstrate dynamic command usage.", cmd_dynamic);
//}}}

//{{{
uint32_t getCycCnt() {
  return DWT->CYCCNT;
  }
//}}}
//{{{
static void usbdUserEvHandler (app_usbd_event_type_t event) {

  switch (event) {
    case APP_USBD_EVT_STOPPED:
      app_usbd_disable();
      break;

    case APP_USBD_EVT_POWER_DETECTED:
      if (!nrf_drv_usbd_is_enabled())
        app_usbd_enable();
      break;

    case APP_USBD_EVT_POWER_REMOVED:
      app_usbd_stop();
      break;

    case APP_USBD_EVT_POWER_READY:
      app_usbd_start();
      break;

    default:
      break;
    }
  }
//}}}
//{{{
static void usbdInit() {

  static const app_usbd_config_t usbdConfig = {
    .ev_handler = app_usbd_event_execute,
    .ev_state_proc = usbdUserEvHandler
    };
  APP_ERROR_CHECK (app_usbd_init (&usbdConfig));

  app_usbd_class_inst_t const* classCdcAcm = app_usbd_cdc_acm_class_inst_get (&nrf_cli_cdc_acm);
  APP_ERROR_CHECK (app_usbd_class_append (classCdcAcm));

  if (USBD_POWER_DETECTION)
    APP_ERROR_CHECK (app_usbd_power_events_enable());
  else {
    NRF_LOG_INFO ("No USB power detection enabled\r\nStarting USB now");
    app_usbd_enable();
    app_usbd_start();
    }

  // Give some time for the host to enumerate and connect to the USB CDC port */
  nrf_delay_ms (1000);
  }
//}}}

int main() {

  // init clocks
  if (USE_CYCCNT_TIMESTAMP_FOR_LOG) {
    //{{{  ccycnt log timestamp
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    APP_ERROR_CHECK (NRF_LOG_INIT (getCycCnt, 64000000));
    }
    //}}}
  else
    APP_ERROR_CHECK (NRF_LOG_INIT (app_timer_cnt_get));
  APP_ERROR_CHECK (nrf_drv_clock_init());
  nrf_drv_clock_lfclk_request (NULL);

  bsp_board_init (BSP_INIT_LEDS);
  bsp_board_led_on (BSP_BOARD_LED_3);
  //{{{  init appTimer
  APP_ERROR_CHECK (app_timer_init());
  APP_ERROR_CHECK (app_timer_create (&m_timer_0, APP_TIMER_MODE_REPEATED, timerHandle));
  APP_ERROR_CHECK (app_timer_start (m_timer_0, APP_TIMER_TICKS(1000), NULL));
  //}}}

  // init cli
  APP_ERROR_CHECK (nrf_cli_init (&mCliCdcAcm, NULL, true, true, NRF_LOG_SEVERITY_INFO));
  APP_ERROR_CHECK (nrf_cli_init (&mCliRtt, NULL, true, true, NRF_LOG_SEVERITY_INFO));
  usbdInit();

  APP_ERROR_CHECK (fds_init());
  nrf_log_config_load();

  // start cli
  APP_ERROR_CHECK (nrf_cli_start (&mCliCdcAcm));
  APP_ERROR_CHECK (nrf_cli_start (&mCliRtt));

  // start log flash
  APP_ERROR_CHECK (nrf_log_backend_flash_init (&nrf_fstorage_nvmc));
  APP_ERROR_CHECK_BOOL (nrf_log_backend_add (&mFlashLogBackend, NRF_LOG_SEVERITY_WARNING) >= 0);
  nrf_log_backend_enable (&mFlashLogBackend);
  APP_ERROR_CHECK_BOOL (nrf_log_backend_add (&mCrashLogBackend, NRF_LOG_SEVERITY_INFO) >= 0);
  nrf_log_backend_enable (&mCrashLogBackend);

  APP_ERROR_CHECK (nrf_mpu_init());
  APP_ERROR_CHECK (nrf_stack_guard_init());

  NRF_LOG_INFO ("cliUsb "__TIME__" "__DATE__);

  while (true) {
    UNUSED_RETURN_VALUE (NRF_LOG_PROCESS());
    #if APP_USBD_CONFIG_EVENT_QUEUE_ENABLE
      while (app_usbd_event_queue_process()) {
        }
    #endif
    nrf_cli_process (&mCliCdcAcm);
    nrf_cli_process (&mCliRtt);
    }
  }
