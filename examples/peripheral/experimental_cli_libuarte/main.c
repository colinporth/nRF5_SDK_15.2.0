//{{{  includes
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "app_timer.h"
#include "app_error.h"
#include "app_util.h"

#include "boards.h"
#include "led_softblink.h"

#include "nrf_cli.h"
#include "nrf_cli_types.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_cli_libuarte.h"

#include "nrf_drv_clock.h"
//}}}

static uint32_t m_counter;
static bool m_counter_active = false;

#define CLI_EXAMPLE_LOG_QUEUE_SIZE 4
NRF_CLI_LIBUARTE_DEF (m_cli_libuarte_transport, 256, 256);
NRF_CLI_DEF (m_cli_libuarte, "libcli$ ", &m_cli_libuarte_transport.transport, '\r', CLI_EXAMPLE_LOG_QUEUE_SIZE);

APP_TIMER_DEF (m_timer_0);
//{{{
static void timer_handle (void * p_context) {

  if (m_counter_active) {
    m_counter++;
    NRF_LOG_RAW_INFO("counter = %d\r\n", m_counter);
    }
  }
//}}}

//{{{
/* Command handlers */
static void cmd_print_param (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  for (size_t i = 1; i < argc; i++)
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "argv[%d] = %s\r\n", i, argv[i]);
  }
//}}}
//{{{
static void cmd_print_all (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  for (size_t i = 1; i < argc; i++)
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "%s ", argv[i]);
  nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "\r\n");
  }
//}}}
//{{{
static void cmd_print (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  ASSERT(p_cli);
  ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

  if ((argc == 1) || nrf_cli_help_requested(p_cli)) {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
    }

  if (argc != 2) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\r\n", argv[0], argv[1]);
  }
//}}}
//{{{
NRF_CLI_CREATE_STATIC_SUBCMD_SET (m_sub_print) {

  NRF_CLI_CMD (all,   NULL, "Print all entered parameters.", cmd_print_all),
  NRF_CLI_CMD (param, NULL, "Print each parameter in new line.", cmd_print_param),
  NRF_CLI_SUBCMD_SET_END
  };
//}}}
NRF_CLI_CMD_REGISTER (print, &m_sub_print, "print", cmd_print);

//{{{
static void cmd_counter_start (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if (argc != 1) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  m_counter_active = true;
  }
//}}}
//{{{
static void cmd_counter_stop (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if (argc != 1) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  m_counter_active = false;
  }
//}}}
//{{{
static void cmd_counter_reset (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  if (argc != 1) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  m_counter = 0;
  }
//}}}
//{{{
static void cmd_counter (nrf_cli_t const* p_cli, size_t argc, char** argv) {

  ASSERT(p_cli);
  ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

  /* Extra defined dummy option */
  static const nrf_cli_getopt_option_t opt[] = { NRF_CLI_OPT( "--test", "-t", "dummy option help string" ) };

  if ((argc == 1) || nrf_cli_help_requested(p_cli)) {
    nrf_cli_help_print(p_cli, opt, ARRAY_SIZE(opt));
    return;
    }

  if (argc != 2) {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
    return;
    }

  if (!strcmp(argv[1], "-t") || !strcmp(argv[1], "--test")) {
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Dummy test option.\r\n");
    return;
    }

  /* subcommands have their own handlers and they are not processed here */
  nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\r\n", argv[0], argv[1]);
  }
//}}}
//{{{
NRF_CLI_CREATE_STATIC_SUBCMD_SET (m_sub_counter) {
  NRF_CLI_CMD(reset,  NULL, "Reset seconds counter.",  cmd_counter_reset),
  NRF_CLI_CMD(start,  NULL, "Start seconds counter.",  cmd_counter_start),
  NRF_CLI_CMD(stop,   NULL, "Stop seconds counter.",   cmd_counter_stop),
  NRF_CLI_SUBCMD_SET_END
  };
//}}}
NRF_CLI_CMD_REGISTER (counter, &m_sub_counter, "Display seconds on terminal screen", cmd_counter);

//{{{
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
//}}}
NRF_CLI_CMD_REGISTER (nordic, NULL, "Print Nordic Semiconductor logo.", cmd_nordic);

int main() {

  APP_ERROR_CHECK (NRF_LOG_INIT (app_timer_cnt_get));

  APP_ERROR_CHECK (nrf_drv_clock_init());
  nrf_drv_clock_lfclk_request (NULL);

  APP_ERROR_CHECK (app_timer_init());
  APP_ERROR_CHECK (app_timer_create (&m_timer_0, APP_TIMER_MODE_REPEATED, timer_handle));
  APP_ERROR_CHECK (app_timer_start (m_timer_0, APP_TIMER_TICKS (1000), NULL));

  bsp_board_init (BSP_INIT_LEDS);
  const led_sb_init_params_t leds = LED_SB_INIT_DEFAULT_PARAMS (LEDS_MASK);
  APP_ERROR_CHECK (led_softblink_init (&leds));
  APP_ERROR_CHECK (led_softblink_start (LEDS_MASK));

  // cli
  cli_libuarte_config_t libuarte_config;
  libuarte_config.tx_pin = TX_PIN_NUMBER;
  libuarte_config.rx_pin = RX_PIN_NUMBER;
  libuarte_config.baudrate = NRF_UARTE_BAUDRATE_115200;
  libuarte_config.parity = NRF_UARTE_PARITY_EXCLUDED;
  libuarte_config.hwfc = NRF_UARTE_HWFC_DISABLED;
  APP_ERROR_CHECK (nrf_cli_init (&m_cli_libuarte, &libuarte_config, true, true, NRF_LOG_SEVERITY_INFO));
  APP_ERROR_CHECK (nrf_cli_start (&m_cli_libuarte));

  NRF_LOG_RAW_INFO ("libcli - built "__TIME__" " __DATE__"\r\n");

  while (true) {
    NRF_LOG_PROCESS();
    nrf_cli_process (&m_cli_libuarte);
    }
  }
