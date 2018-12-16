#pragma once

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

//{{{
/**
 * @brief Function for generating an invalid function pointer HardFault.
 *
 * Function tries to jump into illegal address.
 */
static inline void hardfault_genhf_invalid_fp(void);
//}}}
//{{{
/**
 * @brief Function for generating an undefined instruction HardFault.
 *
 * This function places the value in the code area that is not the legal instruction.
 */
static inline void hardfault_genhf_undefined_instr(void);
//}}}
//{{{
/**
 * @brief Function for generating an unaligned LDM access HardFault.
 *
 * This function generates fault exception loading values from an unaligned address.
 */
static inline void hardfault_genhf_ldm_align(void);
//}}}

//{{{
static inline void hardfault_genhf_invalid_fp(void)
{
    __ASM volatile(
    "    .syntax unified  \n"
    "    movs r0, #0 \n"
    "    blx  r0     \n"
     );
}
//}}}
//{{{
static inline void hardfault_genhf_undefined_instr(void)
{
    __ASM volatile(
    "    .hword 0xf123 \n"
    "    .hword 0x4567 \n"
    );
}
//}}}
//{{{
static inline void hardfault_genhf_ldm_align(void)
{
    __ASM volatile(
    "    .syntax unified  \n"
    "    movs r0,  #1      \n"
    "    ldm  r0!, {r1-r2} \n"
    );
}
//}}}

//{{{
#ifdef __cplusplus
}
#endif
//}}}
