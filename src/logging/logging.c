#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "crypto/crypto.h"
#include "global/common.h"
#include "global/config.h"
#include "hal/hal_storage.h"
#include "hal/hal_time.h"
#include "logging/logging.h"

struct log_stream_t
{
    FILE* file;
};

static void
compute_hmac(log_record_t* record)
{
    memset(record->hmac, 0, sizeof(record->hmac));
    compute_internal_hmac((uint8_t*) record, sizeof(log_record_t) - sizeof(record->hmac),
                          record->hmac, sizeof(record->hmac));
}

static bool
validate_hmac(const log_record_t* record)
{
    log_record_t temp = *record;
    memset(temp.hmac, 0, sizeof(temp.hmac));
    uint8_t temp_mac[LOG_HMAC_SIZE];

    if (compute_internal_hmac((uint8_t*) &temp, sizeof(log_record_t) - sizeof(temp.hmac), temp_mac,
                              sizeof(temp_mac)) != STATUS_OK)
    {
        return false;
    }

    return secure_compare(record->hmac, temp_mac, sizeof(temp_mac)) == STATUS_OK;
}

status_t
log_init(void)
{
    log_stream_t stream;
    log_record_t rec;
    size_t       log_size = 0;
    status_t     status   = STATUS_OK;

    status = hal_storage_log_get_size(&log_size);
    if ((STATUS_OK == status) && (log_size > LOG_MAX_SIZE_BYTES))
    {
        status = STATUS_ERR_LOG_FULL;
    }

    if (STATUS_OK == status)
    {
        if (!hal_log_stream_open(&stream))
        {
            status = STATUS_ERR_STORAGE;
        }
        else
        {
            while (hal_log_stream_next(&stream, &rec))
            {
                // pass
            }
            hal_log_stream_close(&stream);
        }
    }

    return status;
}

status_t
log_write(log_event_t type, const uint8_t* payload, size_t payload_len)
{
    if ((payload_len > 0 && !payload) || payload_len > LOG_MAX_PAYLOAD)
    {
        return STATUS_ERR_INPUT;
    }
    const uint8_t sync_start_byte = 0xA5;
    log_record_t  rec             = {
                     .sync_byte   = sync_start_byte,
                     .data_length = sizeof(uint32_t) + sizeof(uint8_t) + payload_len,
                     .timestamp   = hal_get_timestamp(),
                     .type        = type,
    };

    memcpy_s(rec.payload, LOG_MAX_PAYLOAD, payload, payload_len);
    compute_hmac(&rec);

    return hal_storage_log_append((const uint8_t*) &rec, sizeof(rec));
}

void
log_dump(void)
{
    printf("=== LOG DUMP BEGIN ===\n");

    log_stream_t stream;
    if (!hal_log_stream_open(&stream))
    {
        printf("Failed to open log stream.\n");
        return;
    }

    log_record_t rec;
    unsigned     index = 0;

    while (hal_log_stream_next(&stream, &rec))
    {
        bool valid = validate_hmac(&rec);

        printf("Entry %u:\n", index);
        printf("  HMAC   : %s\n", valid ? "VALID" : "INVALID");
        printf("  Time   : %" PRIu32 "\n", rec.timestamp);
        printf("  Type   : %u\n", rec.type);
        printf("  Length : %u\n", rec.data_length);
        printf("  Payload: ");

        size_t payload_len = 0;
        size_t header_size = sizeof(rec.timestamp) + sizeof(rec.type);

        if (rec.data_length > header_size)
        {
            payload_len = rec.data_length - header_size;
        }
        else
        {
            payload_len = 0;
        }

        if (payload_len > LOG_MAX_PAYLOAD)
        {
            payload_len = LOG_MAX_PAYLOAD;
        }

        for (size_t i = 0; i < payload_len; ++i)
        {
            printf("%02X ", rec.payload[i]);
        }

        printf("\n");

        index++;
    }

    hal_log_stream_close(&stream);
    printf("=== LOG DUMP END ===\n");
}
