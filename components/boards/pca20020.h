#pragma once
//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}
#include "nrf_gpio.h"

#define OSC_XL1                  0
#define OSC_XL2                  1
#define ANA_DIG0                 2
#define ANA_DIG1                 3
#define ANA_DIG2                 4
#define IOEXT_OSCIO              5
#define MPU_INT                  6
#define TWI_SDA                  7
#define TWI_SCL                  8
#define NFC1                     9
#define NFC2                     10
#define BUTTON_1                 11
#define LIS_INT1                 12
#define USB_DETECT               13
#define TWI_SDA_EXT              14
#define TWI_SCL_EXT              15
#define SX_RESET                 16
#define BAT_CHG_STAT             17
#define MOS_1                    18
#define MOS_2                    19
#define MOS_3                    20
#define MOS_4                    21
#define CCS_INT                  22
#define LPS_INT                  23
#define HTS_INT                  24
#define MIC_DOUT                 25
#define MIC_CLK                  26
#define SPEAKER                  27
#define BATTERY                  28
#define SPK_PWR_CTRL             29
#define VDD_PWR_CTRL             30
#define BH_INT                   31
#define NRF_NUM_GPIO_PINS        32

// Button config
#define BUTTON_START             BUTTON_1
#define BUTTON_STOP              BUTTON_1
#define BUTTON_PULL              NRF_GPIO_PIN_PULLUP
#define BUTTONS_ACTIVE_STATE     0
#define BUTTONS_LIST             { BUTTON_1 }
#define BUTTONS_NUMBER           1

// Leds config
#define LED_START      MOS_1
#define LED_1          MOS_1
#define LED_2          MOS_2
#define LED_3          MOS_3
#define LED_4          MOS_4
#define LED_STOP       MOS_4
#define LEDS_ACTIVE_STATE 1
#define LEDS_INV_MASK  LEDS_MASK
#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }
#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4
#define LEDS_NUMBER    4

// IO expander pins
#define SX_IOEXT_NUM_PINS        16

#define SX_IOEXT_0               0
#define SX_IOEXT_1               1
#define SX_IOEXT_2               2
#define SX_IOEXT_3               3
#define SX_BAT_MON_EN            4
#define SX_LIGHTWELL_G           5
#define SX_LIGHTWELL_B           6
#define SX_LIGHTWELL_R           7
#define SX_MPU_PWR_CTRL          8
#define SX_MIC_PWR_CTRL          9
#define SX_CCS_PWR_CTRL          10
#define SX_CCS_RESET             11
#define SX_CCS_WAKE              12
#define SX_SENSE_LED_R           13
#define SX_SENSE_LED_G           14
#define SX_SENSE_LED_B           15

// Uart configuration for testing
#define TX_PIN_NUMBER            ANA_DIG0
#define RX_PIN_NUMBER            ANA_DIG1
#define CTS_PIN_NUMBER           0
#define RTS_PIN_NUMBER           0
#define HWFC                     false

//{{{
#ifdef __cplusplus
}
#endif
//}}}
