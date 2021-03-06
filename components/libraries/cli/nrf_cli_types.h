#pragma once

#include <inttypes.h>
#include "sdk_config.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

#if (NRF_CLI_CMD_BUFF_SIZE > 65535)
  typedef uint32_t nrf_cli_cmd_len_t;
#elif (NRF_CLI_CMD_BUFF_SIZE > 255)
  typedef uint16_t nrf_cli_cmd_len_t;
#else
  typedef uint8_t nrf_cli_cmd_len_t;
#endif

typedef enum {
  NRF_CLI_VT100_COLOR_DEFAULT,
  NRF_CLI_VT100_COLOR_BLACK,
  NRF_CLI_VT100_COLOR_RED,
  NRF_CLI_VT100_COLOR_GREEN,
  NRF_CLI_VT100_COLOR_YELLOW,
  NRF_CLI_VT100_COLOR_BLUE,
  NRF_CLI_VT100_COLOR_MAGENTA,
  NRF_CLI_VT100_COLOR_CYAN,
  NRF_CLI_VT100_COLOR_WHITE,

  VT100_COLOR_END
  } nrf_cli_vt100_color_t;

typedef struct {
  nrf_cli_vt100_color_t col;      // text color
  nrf_cli_vt100_color_t bgcol;    // background color
  } nrf_cli_vt100_colors_t;

typedef struct {
  nrf_cli_cmd_len_t cur_x;        // horizontal cursor position in edited command line
  nrf_cli_cmd_len_t cur_x_end;    // horizontal cursor position at the end of command
  nrf_cli_cmd_len_t cur_y;        // vertical cursor position in edited command
  nrf_cli_cmd_len_t cur_y_end;    // vertical cursor position at the end of command
  nrf_cli_cmd_len_t terminal_hei; // terminal screen height
  nrf_cli_cmd_len_t terminal_wid; // terminal screen width
  uint8_t name_len;               // console name length
  } nrf_cli_multiline_cons_t;

typedef struct {
  nrf_cli_multiline_cons_t cons;
  nrf_cli_vt100_colors_t col;
  nrf_cli_cmd_len_t printed_cmd;  // printed commands counter
  } nrf_cli_vt100_ctx_t;

//{{{
#ifdef __cplusplus
}
#endif
//}}}
