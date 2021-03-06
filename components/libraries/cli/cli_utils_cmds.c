#include "nrf_cli.h"
#include "nrf_log.h"

//{{{
static void cmd_reset (nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    NVIC_SystemReset();
}
//}}}
//{{{
static void cmd_error (nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
}
//}}}
//{{{
static void cmd_app_size (nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    nrf_cli_fprintf(p_cli,
                    NRF_CLI_NORMAL,
                    "Application address:%d (0x%08X), size: %d (0x%08X)\r\n",
                    CODE_START,
                    CODE_START,
                    CODE_SIZE,
                    CODE_SIZE);
}
//}}}

//{{{
static void cmd_log_msg_error (nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    switch (argc-1)
    {
        case 0:
            NRF_LOG_ERROR("test error message");
            break;
        case 1:
            NRF_LOG_ERROR("test error message: %d", strtol(argv[1], NULL, 10));
            break;
        case 2:
            NRF_LOG_ERROR("test error message: %d %d", strtol(argv[1], NULL, 10),
                                                       strtol(argv[2], NULL, 10));
            break;
        default:
            NRF_LOG_ERROR("test error message with more than 3 arguments");
            break;
    }
}
//}}}
//{{{
static void cmd_log_msg_warning (nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    switch (argc-1)
    {
        case 0:
            NRF_LOG_WARNING("test warning message");
            break;
        case 1:
            NRF_LOG_WARNING("test warning message: %d", strtol(argv[1], NULL, 10));
            break;
        case 2:
            NRF_LOG_WARNING("test warning message: %d %d", strtol(argv[1], NULL, 10),
                                                       strtol(argv[2], NULL, 10));
            break;
        default:
            NRF_LOG_WARNING("test warning message with more than 3 arguments");
            break;
    }
}
//}}}

NRF_CLI_CMD_REGISTER (reset, NULL, "System reset.", cmd_reset);
NRF_CLI_CMD_REGISTER (error, NULL, "Trigger error.", cmd_error);
NRF_CLI_CMD_REGISTER (app_size, NULL, "Print application size.", cmd_app_size);

NRF_CLI_CREATE_STATIC_SUBCMD_SET (m_sub_log_msg) {
  NRF_CLI_CMD (error,   NULL, "Error log message with parameters", cmd_log_msg_error),
  NRF_CLI_CMD (warning, NULL, "Warning log message with parameters", cmd_log_msg_warning),
  NRF_CLI_SUBCMD_SET_END
  };

NRF_CLI_CMD_REGISTER (log_msg, &m_sub_log_msg, "Trigger log message with decimal arguments", NULL);
