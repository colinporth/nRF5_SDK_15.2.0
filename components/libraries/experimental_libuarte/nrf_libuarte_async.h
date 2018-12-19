#pragma once

#include <stdint.h>
#include "sdk_errors.h"
#include <hal/nrf_uarte.h>

/** @brief Types of libuarte driver events. */
typedef enum {
  NRF_LIBUARTE_ASYNC_EVT_RX_DATA, ///< Requested TX transfer completed.
  NRF_LIBUARTE_ASYNC_EVT_TX_DONE, ///< Requested RX transfer completed.
  NRF_LIBUARTE_ASYNC_EVT_ERROR    ///< Error reported by UARTE peripheral.
  } nrf_libuarte_async_evt_type_t;

/** @brief Structure for libuarte async transfer completion event. */
typedef struct {
  uint8_t* p_data; ///< Pointer to memory used for transfer.
  size_t   length; ///< Number of bytes transfered.
  } nrf_libuarte_async_data_t;

/** @brief Structure for libuarte async configuration. */
typedef struct {
  uint32_t             rx_pin;     ///< RXD pin number.
  uint32_t             tx_pin;     ///< TXD pin number.
  uint32_t             cts_pin;    ///< CTS pin number.
  uint32_t             rts_pin;    ///< RTS pin number.
  uint32_t             timeout_us; ///< Receiver timeout in us unit.
  nrf_uarte_hwfc_t     hwfc;       ///< Flow control configuration.
  nrf_uarte_parity_t   parity;     ///< Parity configuration.
  nrf_uarte_baudrate_t baudrate;   ///< Baudrate.
  } nrf_libuarte_async_config_t;

/** @brief Structure for libuarte error event. */
typedef struct {
  nrf_libuarte_async_evt_type_t type; ///< Event type.
  union {
    nrf_libuarte_async_data_t rxtx;   ///< RXD/TXD data.
    } data;                           ///< Union with data.
  } nrf_libuarte_async_evt_t;

//{{{
/**
 * @brief Interrupt event handler.
 *
 * @param[in] p_evt  Pointer to event structure. Event is allocated on the stack so it is available
 *                   only within the context of the event handler.
 */
typedef void (*nrf_libuarte_async_evt_handler)(nrf_libuarte_async_evt_t * p_evt);
//}}}

//{{{
/**
 * @brief Function for initializing the libuarte async library.
 *
 * @param[in] p_config     Pointer to the structure with initial configuration.
 * @param[in] evt_handler  Event handler provided by the user. Must not be NULL.
 *
 * @return NRF_SUCCESS when properly initialized. NRF_ERROR_INTERNAL otherwise.
 */
ret_code_t nrf_libuarte_async_init (nrf_libuarte_async_config_t const * p_config,
                                   nrf_libuarte_async_evt_handler      evt_handler);
//}}}
//{{{
/** @brief Function for uninitializing the libuarte async library */
void nrf_libuarte_async_uninit();

/**
 * @brief Funtrion for setting buffer and its size for received data.
 *
 * @param chunk_size  Number of bytes in chunk of data
 */
void nrf_libuarte_async_enable(size_t chunk_size);
//}}}

//{{{
/**
 * @brief Function for sending data asynchronously over UARTE.
 *
 * @param[in] p_data  Pointer to data.
 * @param[in] length  Number of bytes to send. Maximum possible length is
 *                    dependent on the used SoC (see the MAXCNT register
 *                    description in the Product Specification). The library
 *                    checks it with assertion.
 *
 * @retval NRF_ERROR_BUSY      Data is transferring.
 * @retval NRF_ERROR_INTERNAL  Error during configuration.
 * @retval NRF_SUCCESS         Buffer set for sending.
 */
ret_code_t nrf_libuarte_async_tx (uint8_t * p_data, size_t length);
//}}}
//{{{
/**
 * @brief Function for deallocating received buffer data.
 *
 * @param[in] p_data  Pointer to data.
 * @param[in] length  Number of bytes to free.
 */
void nrf_libuarte_async_rx_free (uint8_t * p_data, size_t length);
//}}}
