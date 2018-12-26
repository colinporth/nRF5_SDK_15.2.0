#pragma once

#include "nrf_log_backend_interface.h"
#include "nrf_fstorage.h"
#include "nrf_log_internal.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

/** @brief Flashlog logger backend API. */
extern const nrf_log_backend_api_t nrf_log_backend_flashlog_api;

/** @brief Crashlog logger backend API. */
extern const nrf_log_backend_api_t nrf_log_backend_crashlog_api;

typedef struct {
  nrf_log_backend_t      backend;
  } nrf_log_backend_flashlog_t;

typedef struct {
  nrf_log_backend_t      backend;
  } nrf_log_backend_crashlog_t;

/** @brief Macro for creating an instance of the flashlog logger backend. */
#define NRF_LOG_BACKEND_FLASHLOG_DEF(_name) NRF_LOG_BACKEND_DEF(_name, nrf_log_backend_flashlog_api, NULL)

/** @brief Macro for creating an instance of the crashlog logger backend. */
#define NRF_LOG_BACKEND_CRASHLOG_DEF(_name) NRF_LOG_BACKEND_DEF(_name, nrf_log_backend_crashlog_api, NULL)

//{{{
/**
 * @brief Function for initializing the flash logger backend.
 * Flash logger backend consists of two logical backends: flashlog and crashlog. Since both
 * backends write to the same flash area, the initialization is common.
 * @param p_fs_api fstorage API to be used.
 * @return NRF_SUCCESS or error code returned by @ref nrf_fstorage_init.
 */
ret_code_t nrf_log_backend_flash_init (nrf_fstorage_api_t const* p_fs_api);
//}}}
//{{{
/**
 * @brief Function for getting a log entry stored in flash.
 * Log messages stored in flash can be read one by one starting from the oldest one.
 * @param[in, out] p_token   Token reused between consecutive readings of log entries.
 *                           Token must be set to 0 to read the first entry.
 * @param[out]     pp_header Pointer to the entry header.
 * @param[out]     pp_data   Pointer to the data part of the entry (arguments or data in case of hexdump).
 * @retval NRF_SUCCESS             Entry was successfully read.
 * @retval NRF_ERROR_NOT_SUPPORTED fstorage API does not support direct reading.
 * @retval NRF_ERROR_NOT_FOUND     Entry not found. Last entry was already reached or area is empty.
 */
ret_code_t nrf_log_backend_flash_next_entry_get (uint32_t* p_token, nrf_log_header_t** pp_header, uint8_t** pp_data);
//}}}
ret_code_t nrf_log_backend_flash_erase();

//{{{
#ifdef __cplusplus
}
#endif
//}}}
