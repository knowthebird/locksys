// src/global/user.h
#ifndef USER_H
#define USER_H

#include <stdbool.h>

#include "global/common.h"
#include "global/config.h"

// User flags
#define USER_FLAG_IS_ENABLED 0x01
#define USER_FLAG_IS_ADMIN 0x02
#define USER_FLAG_IS_LOCKED 0x04
#define USER_FLAG_FORCE_PASS_RESET 0x08
#define USER_FLAG_RESERVED1 0x10
#define USER_FLAG_RESERVED2 0x20
#define USER_FLAG_RESERVED3 0x40
#define USER_FLAG_RESERVED4 0x80

typedef struct __attribute__((packed))
{
    char     username[MAX_USERNAME_LEN];
    uint8_t  password_hmac[LOCKSYS_HASH_SIZE];
    uint8_t  failed_attempts_since_login;
    uint32_t last_attempt_timestamp;
    uint32_t created_timestamp;
    uint32_t password_last_set;
    uint8_t  user_flags;
    uint8_t  reserved[2]; // padding/future use
    uint8_t  record_hmac[LOCKSYS_HASH_SIZE];
} user_record_t;

status_t
user_find_by_username(const char* name, uint8_t* out_index, user_record_t* out_user);

status_t
user_record_compute_hmac(user_record_t* user);

status_t
user_record_validate_hmac(const user_record_t* user);

status_t
user_add(const char* username, const char* password, uint8_t is_admin);

status_t
user_get_is_admin(const char* username, bool* out_is_admin);

#endif // USER_H
