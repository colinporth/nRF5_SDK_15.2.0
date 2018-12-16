#include "sdk_common.h"

#if NRF_MODULE_ENABLED(HARDFAULT_HANDLER)
  #include <stdint.h>
  #include "compiler_abstraction.h"

  extern void HardFault_c_handler(uint32_t *);

  void HardFault_Handler(void) __attribute__(( naked ));
  void HardFault_Handler(void) {
    __ASM volatile(
    "   tst lr, #4                              \n"

    /* PSP is quite simple and does not require additional handler */
    "   itt ne                                  \n"
    "   mrsne r0, psp                           \n"
    /* Jump to the handler, do not store LR - returning from handler just exits exception */
    "   bne  HardFault_Handler_Continue         \n"

    /* Processing MSP requires stack checking */
    "   mrs r0, msp                             \n"

    "   ldr   r1, =__StackTop                   \n"
    "   ldr   r2, =__StackLimit                 \n"

    /* MSP is in the range of the stack area */
    "   cmp   r0, r1                            \n"
    "   bhi   HardFault_MoveSP                  \n"
    "   cmp   r0, r2                            \n"
    "   bhi   HardFault_Handler_Continue        \n"

    "HardFault_MoveSP:                          \n"
    "   mov   sp, r1                            \n"
    "   mov   r0, #0                            \n"

    "HardFault_Handler_Continue:                \n"
    "   ldr r3, =%0                             \n"
    "   bx r3                                   \n"
    "   .ltorg                                  \n"
    : : "X"(HardFault_c_handler)
    );
  }

#endif //NRF_MODULE_ENABLED(HARDFAULT_HANDLER)
