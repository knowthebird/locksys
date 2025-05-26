#ifndef COMMON_H
#define COMMON_H

#include "global/config.h"
#include <stdint.h>

typedef struct
{
    uint8_t  failed_attempts;   // Global failed login count
    uint32_t last_attempt_time; // Timestamp of last failed attempt
    uint8_t  user_count;
    uint8_t  reserved[2]; // Reserved for future use (pads to 16 bytes)
    uint8_t  hmac[LOCKSYS_HASH_SIZE];
} system_state_t;

//  Common status code enum
typedef enum
{
    STATUS_OK = 0,
    STATUS_ERR_INPUT,
    STATUS_ERR_INTERNAL,
    STATUS_ERR_PERM_LOCKED,
    STATUS_ERR_TIMEOUT,
    STATUS_ERR_AUTH,
    STATUS_ERR_TAMPER,
    STATUS_ERR_UNINITIALIZED,
    STATUS_ERR_LOG_FULL,
    STATUS_ERR_STORAGE,
    STATUS_ERR_NOT_FOUND,
    STATUS_ERR_THROTTLED,
} status_t;

//  Add other shared types/macros here as needed

status_t
system_state_compute_hmac(system_state_t* state);

status_t
system_state_validate_hmac(const system_state_t* state);

#endif //  COMMON_H