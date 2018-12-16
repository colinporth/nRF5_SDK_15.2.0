#pragma once

#include <stdint.h>
#include "nrf_cli.h"
#include "bsp.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

ret_code_t bsp_cli_init(bsp_event_callback_t callback);

//{{{
#ifdef __cplusplus
}
#endif
//}}}
