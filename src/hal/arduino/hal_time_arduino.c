#include "global/config.h"

#if defined(PLATFORM_ARDUINO)

#include "hal/hal_time.h"

#warning "Unsupported platform for hal_time.c, using dummy functions."

// Simple counter to simulate a timestamp
static uint32_t fake_timestamp = 0;

uint32_t
hal_get_timestamp(void)
{
    // Return and increment dummy time value
    return fake_timestamp++;
}

#endif
