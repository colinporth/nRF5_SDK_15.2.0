//{{{  includes
#include "sdk_common.h"
#include "compiler_abstraction.h"
#include "app_error.h"
//}}}

#if defined (__CORTEX_M) && (__CORTEX_M == 0x04)
  void app_error_handler(ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name) __attribute__(( naked ));

  void app_error_handler(ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name) {

    __ASM volatile(

    "push {lr}                      \n"

    /* reserve space on stack for error_info_t struct - preserve 8byte stack aligment */
    "sub sp, sp, %0                 \n"

    /* prepare error_info_t struct */
    "str r0, [sp, %1]               \n"
    "str r1, [sp, %3]               \n"
    "str r2, [sp, %2]               \n"

    /* prepare arguments and call function: app_error_fault_handler */
    "ldr r0, =%4                    \n"
    "mov r1, lr                     \n"
    "mov r2, sp                     \n"
    "bl  %5                         \n"

    /* release stack */
    "add sp, sp, %0                 \n"

    "pop {pc}                       \n"
    ".ltorg                         \n"

    : /* Outputs */
    : /* Inputs */
    "I" (APP_ERROR_ERROR_INFO_SIZE_ALIGNED_8BYTE),
    "I" (APP_ERROR_ERROR_INFO_OFFSET_ERR_CODE),
    "I" (APP_ERROR_ERROR_INFO_OFFSET_P_FILE_NAME),
    "I" (APP_ERROR_ERROR_INFO_OFFSET_LINE_NUM),
    "X" (NRF_FAULT_ID_SDK_ERROR),
    "X" (app_error_fault_handler)
    : /* Clobbers */
    "r0", "r1", "r2"
    );
  }
#elif defined(__CORTEX_M) && (__CORTEX_M == 0x00)
  /* NRF51 implementation is currently not supporting PC readout */
  void app_error_handler(ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name)
  {
      error_info_t error_info = {
          .line_num    = line_num,
          .p_file_name = p_file_name,
          .err_code    = error_code,
      };
      app_error_fault_handler(NRF_FAULT_ID_SDK_ERROR, 0, (uint32_t)(&error_info));

      UNUSED_VARIABLE(error_info);
  }
#else
  #error Architecture not supported
#endif
