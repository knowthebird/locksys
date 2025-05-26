#ifndef LOGGING_H
#define LOGGING_H

#include "global/common.h"
#include "global/config.h"
#include <stdbool.h>
#include <stddef.h> // for size_t
#include <stdint.h>

// --- Log event codes ---
typedef enum
{
    EVENT_APPLICATION_START   = 1,
    EVENT_REQUEST_TO_UNLOCK   = 2,
    EVENT_UNLOCKING_DEVICE    = 3,
    EVENT_LOCKING_DEVICE      = 4,
    EVENT_UNLOCK_CHECK_FAILED = 5,
    EVENT_REQUEST_PASS_CHANGE = 6,
    EVENT_PASS_CHANGE_FAILED  = 7,
    EVENT_PASS_CHANGE_PASSED  = 8,
} log_event_t;

// --- Log record structure ---
struct log_record_t
{
    uint8_t  sync_byte;                // Always 0xA5
    uint16_t data_length;              // timestamp + type + payload
    uint32_t timestamp;                // Event time
    uint8_t  type;                     // Event type (log_event_t or custom)
    uint8_t  payload[LOG_MAX_PAYLOAD]; // Variable content
    uint8_t  hmac[LOG_HMAC_SIZE];      // Integrity check
} __attribute__((packed));

status_t
log_init(void);

// New stream-style logging API
status_t
log_write(log_event_t type, const uint8_t* payload, size_t payload_len);

typedef struct log_record_t log_record_t;

void
log_dump(void);

#endif // LOGGING_H
