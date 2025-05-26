#include "global/config.h"

#if defined(PLATFORM_POSIX)

#include "hal/hal_storage.h"

#warning "Unsupported platform for hal_storage.c, using dummy functions."

#include <stdio.h>
#include <string.h>

#ifdef USE_FIRMWARE_KEY
#include "global/device_key.generated.h"
#endif

status_t
hal_load_device_key(uint8_t* key_buf, size_t key_len)
{
    if (key_buf && key_len > 0)
    {
        key_buf[0] = 0xAB; // example dummy data
    }
    return STATUS_OK;
}

status_t
hal_storage_log_append(const uint8_t* src, size_t len)
{
    (void) src;
    (void) len;
    return STATUS_OK;
}

bool
hal_log_stream_open(log_stream_t* stream)
{
    (void) stream;
    return true;
}

bool
hal_log_stream_next(log_stream_t* stream, struct log_record_t* rec)
{
    (void) stream;
    (void) rec;
    return false; // no records
}

void
hal_log_stream_close(log_stream_t* stream)
{
    (void) stream;
}

status_t
hal_storage_user_get(uint8_t index, user_record_t* out)
{
    if (!out || index >= MAX_USERS)
        return STATUS_ERR_INPUT;
    memset(out, 0, sizeof(user_record_t)); // dummy data for testing
    return STATUS_OK;
}

status_t
hal_storage_user_set(uint8_t index, const user_record_t* in)
{
    if (!in || index >= MAX_USERS)
        return STATUS_ERR_INPUT;
    // could write to EEPROM later
    return STATUS_OK;
}

status_t
hal_storage_get_system_state(system_state_t* out)
{
    if (!out)
        return STATUS_ERR_INPUT;
    memset(out, 0, sizeof(system_state_t)); // dummy
    return STATUS_OK;
}

status_t
hal_storage_set_system_state(system_state_t* in)
{
    if (!in)
        return STATUS_ERR_INPUT;
    return STATUS_OK;
}

status_t
hal_storage_log_get_size(size_t* out_size)
{
    if (!out_size)
        return STATUS_ERR_INPUT;
    *out_size = 0;
    return STATUS_OK;
}

status_t
hal_storage_log_get_size(size_t* out_size)
{
    if (!out_size)
    {
        return STATUS_ERR_INPUT;
    }

    FILE* file = fopen(LOG_STORAGE_FILENAME, "rb");
    if (!file)
    {
        return STATUS_ERR_STORAGE;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return STATUS_ERR_STORAGE;
    }

    long size = ftell(file);
    fclose(file);

    if (size < 0)
    {
        return STATUS_ERR_STORAGE;
    }

    *out_size = (size_t) size;
    return STATUS_OK;
}

#endif
