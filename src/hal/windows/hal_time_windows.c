#include "global/config.h"

#if defined(PLATFORM_WINDOWS)

#include "hal/hal_time.h"
#include <time.h>
#include <windows.h>

uint32_t
hal_get_timestamp(void)
{
    return (uint32_t) time(NULL); // UNIX epoch seconds
}

#endif
