#include "global/config.h"

#if defined(PLATFORM_WINDOWS)

#include "hal/hal_io.h"

#include <stdio.h>
#include <windows.h>

status_t
hal_display(const char* msg)
{
    printf("%s", msg);
    return STATUS_OK;
}

status_t
hal_delay_ms(uint32_t ms)
{
    Sleep(ms);
    return STATUS_OK;
}

char
hal_get_input_char_raw(void)
{
    INPUT_RECORD record;
    DWORD        read;
    HANDLE       hStdin = GetStdHandle(STD_INPUT_HANDLE);
    while (1)
    {
        ReadConsoleInput(hStdin, &record, 1, &read);
        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown)
        {
            return record.Event.KeyEvent.uChar.AsciiChar;
        }
    }
}
char
hal_get_input_char_line(void)
{
    return getchar();
}

status_t
hal_flush_input_line(void)
{
    char ch;
    do
    {
        ch = getchar();
    } while (ch != '\n' && ch != '\r');
    return STATUS_OK;
}

status_t
hal_is_reset_jumper_enabled(bool* is_enabled)
{
    *is_enabled = false;
    return STATUS_OK;
}

status_t
hal_lock_open()
{
    printf("[LOCK] Opening\n");
    return STATUS_OK;
}

status_t
hal_lock_close()
{
    printf("[LOCK] Closed\n");
    return STATUS_OK;
}

#endif
