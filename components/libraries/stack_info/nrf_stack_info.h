#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "compiler_abstraction.h"
#include "app_util.h"

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

// Top (highest) stack address.
#define NRF_STACK_INFO_TOP          ((uint32_t)STACK_TOP)

// Base (lowest) stack address.
#define NRF_STACK_INFO_BASE         ((uint32_t)STACK_BASE)

//{{{
/**
 * @brief Function to get the current stack pointer value.
 *
 * @return      Current stack pointer value.
 */
#define NRF_STACK_INFO_GET_SP()     ((uint32_t)GET_SP())
//}}}

__STATIC_INLINE size_t nrf_stack_info_get_available(void);
__STATIC_INLINE size_t nrf_stack_info_get_depth(void);
__STATIC_INLINE bool nrf_stack_info_overflowed(void);
__STATIC_INLINE bool nrf_stack_info_is_on_stack(void const * const p_address);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION
  //{{{
  /**
   * @brief Calculate the available (free) space on the stack.
   *
   * @return      Number of available bytes on the stack.
   */
   __STATIC_INLINE size_t nrf_stack_info_get_available(void)
  {
      uint32_t sp = NRF_STACK_INFO_GET_SP();
      if (sp > NRF_STACK_INFO_BASE)
      {
          return (size_t)(sp - NRF_STACK_INFO_BASE);
      }

      // Stack overflow
      return 0;
  }
  //}}}
  //{{{
  /**
   * @brief Calculate the current stack depth (occupied space).
   *
   * @return      Current stack depth in bytes.
   */
  __STATIC_INLINE size_t nrf_stack_info_get_depth(void)
  {
      return (size_t)(NRF_STACK_INFO_TOP - NRF_STACK_INFO_GET_SP());
  }
  //}}}
  //{{{
  /**
   * @brief Function for checking if the stack is currently overflowed.
   *
   * @details This function checks if the stack is currently in an overflowed by comparing the stack
   *          pointer with the stack base address. Intended to be used to be used to ease debugging.
   *
   * @return      true if stack is currently overflowed, false otherwise.
   */
   __STATIC_INLINE bool nrf_stack_info_overflowed(void)
   {
      if (NRF_STACK_INFO_GET_SP() < NRF_STACK_INFO_BASE)
      {
          return true;
      }

      return false;
   }
  //}}}
  //{{{
  /**
   * @brief Function for checking if provided address is located in stack space.
   *
   * @param[in]   p_address   Address to be checked.
   *
   * @return      true if address is in stack space, false otherwise.
   */
  __STATIC_INLINE bool nrf_stack_info_is_on_stack(void const * const p_address)
  {
      if (((uint32_t)p_address >= NRF_STACK_INFO_BASE) && ((uint32_t)p_address <  NRF_STACK_INFO_TOP))
      {
          return true;
      }

      return false;
  }
  //}}}
#endif
// SUPPRESS_INLINE_IMPLEMENTATION

//{{{
#ifdef __cplusplus
}
#endif 
//}}}
