#include "global/config.h"

#if defined(PLATFORM_POSIX)

#include "hal_io.h"

#warning "Unsupported platform for hal_io.c, using dummy functions."

status_t
hal_display(const char* msg)
{
    // Display placeholder
    (void) msg;
    return STATUS_OK;
}

status_t
hal_delay_ms(uint32_t ms)
{
    // Delay placeholder
    (void) ms;
    return STATUS_OK;
}

char
hal_get_input_char_raw(void)
{
    // Simulated masked char input
    return '*';
}

char
hal_get_input_char_line(void)
{
    // Simulated visible input
    return 'y';
}

status_t
hal_flush_input_line(void)
{
    return STATUS_OK;
}

status_t
hal_is_reset_jumper_enabled(bool* is_enabled)
{
    if (is_enabled)
    {
        *is_enabled = false;
    }
    return STATUS_OK;
}

status_t
hal_lock_open(void)
{
    // Placeholder for opening a lock
    return STATUS_OK;
}

status_t
hal_lock_close(void)
{
    // Placeholder for closing a lock
    return STATUS_OK;
}

#endif
