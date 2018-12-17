#pragma once

#include "nrf_log_backend_interface.h"
#include "nrf_fprintf.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

void nrf_log_backend_serial_put (nrf_log_backend_t const* p_backend,
                                 nrf_log_entry_t* p_msg, uint8_t* p_buffer, uint32_t length, nrf_fprintf_fwrite tx_func);

//{{{
#ifdef __cplusplus
}
#endif
//}}}
