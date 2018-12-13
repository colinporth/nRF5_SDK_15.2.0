#pragma once

#include "sdk_common.h"
#include "nrf_section.h"
#if NRF_MODULE_ENABLED(NRF_LOG)
  #include "nrf_strerror.h"
  #define NRF_LOG_ERROR_STRING_GET(code) nrf_strerror_get(code)
#else
  #define NRF_LOG_ERROR_STRING_GET(code) ""
#endif

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

/** @brief Severity level for the module.
 *
 * The severity level can be defined in a module to override the default.
 */
#ifndef NRF_LOG_LEVEL
  #define NRF_LOG_LEVEL NRF_LOG_DEFAULT_LEVEL
#endif

/** @brief Initial severity if filtering is enabled.
 */
#ifndef NRF_LOG_INITIAL_LEVEL
  #define NRF_LOG_INITIAL_LEVEL NRF_LOG_LEVEL
#endif

#include "nrf_log_internal.h"

#define NRF_LOG_ERROR(...)                     NRF_LOG_INTERNAL_ERROR(__VA_ARGS__)
#define NRF_LOG_WARNING(...)                   NRF_LOG_INTERNAL_WARNING( __VA_ARGS__)
#define NRF_LOG_INFO(...)                      NRF_LOG_INTERNAL_INFO( __VA_ARGS__)
#define NRF_LOG_DEBUG(...)                     NRF_LOG_INTERNAL_DEBUG( __VA_ARGS__)

#define NRF_LOG_INST_ERROR(p_inst,...)         NRF_LOG_INTERNAL_INST_ERROR(p_inst,__VA_ARGS__)
#define NRF_LOG_INST_WARNING(p_inst,...)       NRF_LOG_INTERNAL_INST_WARNING(p_inst,__VA_ARGS__)
#define NRF_LOG_INST_INFO(p_inst,...)          NRF_LOG_INTERNAL_INST_INFO(p_inst, __VA_ARGS__)
#define NRF_LOG_INST_DEBUG(p_inst,...)         NRF_LOG_INTERNAL_INST_DEBUG(p_inst, __VA_ARGS__)

#define NRF_LOG_RAW_INFO(...)                  NRF_LOG_INTERNAL_RAW_INFO( __VA_ARGS__)

#define NRF_LOG_HEXDUMP_ERROR(p_data, len)   NRF_LOG_INTERNAL_HEXDUMP_ERROR(p_data, len)
#define NRF_LOG_HEXDUMP_WARNING(p_data, len) NRF_LOG_INTERNAL_HEXDUMP_WARNING(p_data, len)
#define NRF_LOG_HEXDUMP_INFO(p_data, len)    NRF_LOG_INTERNAL_HEXDUMP_INFO(p_data, len)
#define NRF_LOG_HEXDUMP_DEBUG(p_data, len)   NRF_LOG_INTERNAL_HEXDUMP_DEBUG(p_data, len)

#define NRF_LOG_HEXDUMP_INST_ERROR(p_inst, p_data, len)   NRF_LOG_INTERNAL_HEXDUMP_INST_ERROR(p_inst, p_data, len)
#define NRF_LOG_HEXDUMP_INST_WARNING(p_inst, p_data, len) NRF_LOG_INTERNAL_HEXDUMP_INST_WARNING(p_inst, p_data, len)
#define NRF_LOG_HEXDUMP_INST_INFO(p_inst, p_data, len)    NRF_LOG_INTERNAL_HEXDUMP_INST_INFO(p_inst, p_data, len)
#define NRF_LOG_HEXDUMP_INST_DEBUG(p_inst, p_data, len)   NRF_LOG_INTERNAL_HEXDUMP_INST_DEBUG(p_inst, p_data, len)

#define NRF_LOG_RAW_HEXDUMP_INFO(p_data, len) NRF_LOG_INTERNAL_RAW_HEXDUMP_INFO(p_data, len)


#define NRF_LOG_PUSH(_str)                   NRF_LOG_INTERNAL_LOG_PUSH(_str)
char const * nrf_log_push(char * const p_str);

#define NRF_LOG_FLOAT_MARKER "%s%d.%02d"
#define NRF_LOG_FLOAT(val) (uint32_t)(((val) < 0 && (val) > -1.0) ? "-" : ""),   \
                           (int32_t)(val),                                       \
                           (int32_t)((((val) > 0) ? (val) - (int32_t)(val)       \
                                                : (int32_t)(val) - (val))*100)

#define NRF_LOG_MODULE_REGISTER() NRF_LOG_INTERNAL_MODULE_REGISTER()

//{{{
#ifdef __cplusplus
}
#endif
//}}}
