#include "sdk_common.h"

#if NRF_MODULE_ENABLED(NRF_LOG) && NRF_MODULE_ENABLED(NRF_LOG_BACKEND_RTT)
  #include "nrf_log_backend_rtt.h"
  #include "nrf_log_backend_serial.h"
  #include "nrf_log_str_formatter.h"
  #include "nrf_log_internal.h"
  #include "nrf_delay.h"
  #include <SEGGER_RTT_Conf.h>
  #include <SEGGER_RTT.h>

  static bool m_host_present;
  static uint8_t m_string_buff[NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE];

  //{{{
  static void serial_tx (void const * p_context, char const * buffer, size_t len)
  {
      if (len)
      {
          uint32_t idx    = 0;
          uint32_t processed;
          uint32_t watchdog_counter = NRF_LOG_BACKEND_RTT_TX_RETRY_CNT;
          do
          {
              processed = SEGGER_RTT_WriteNoLock(0, &buffer[idx], len);
              idx += processed;
              len -= processed;
              if (processed == 0)
              {
                  /* There are two possible reasons for not writing any data to RTT:
                   * - The host is not connected and not reading the data.
                   * - The buffer got full and will be read by the host.
                   * These two situations are distinguished using the following algorithm.
                   * At the begining, the module assumes that the host is active,
                   * so when no data is read, it busy waits and retries.
                   * If, after retrying, the host reads the data, the module assumes that the host is active.
                   * If it fails, the module assumes that the host is inactive and stores that information. On next
                   * call, only one attempt takes place. The host is marked as active if the attempt is successful.
                   */
                  if (!m_host_present)
                  {
                      break;
                  }
                  else
                  {
                      nrf_delay_ms(NRF_LOG_BACKEND_RTT_TX_RETRY_DELAY_MS);
                      watchdog_counter--;
                      if (watchdog_counter == 0)
                      {
                          m_host_present = false;
                          break;
                      }
                  }
              }
              m_host_present = true;
          } while (len);
      }
  }
  //}}}

  //{{{
  static void nrf_log_backend_rtt_put (nrf_log_backend_t const * p_backend,
                                 nrf_log_entry_t * p_msg)
  {
      nrf_log_backend_serial_put(p_backend, p_msg, m_string_buff, NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE, serial_tx);
  }
  //}}}
  //{{{
  static void nrf_log_backend_rtt_flush (nrf_log_backend_t const * p_backend)
  {

  }
  //}}}
  //{{{
  static void nrf_log_backend_rtt_panic_set (nrf_log_backend_t const * p_backend)
  {

  }
  //}}}

  //{{{
  void nrf_log_backend_rtt_init()
  {
      SEGGER_RTT_Init();
  }
  //}}}

  const nrf_log_backend_api_t nrf_log_backend_rtt_api = {
    .put       = nrf_log_backend_rtt_put,
    .flush     = nrf_log_backend_rtt_flush,
    .panic_set = nrf_log_backend_rtt_panic_set,
    };

#endif
