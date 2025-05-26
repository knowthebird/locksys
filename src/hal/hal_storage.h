//  Copyright 2025 Ross Kinard

#ifndef INCLUDE_HAL_STORAGE_H_
#define INCLUDE_HAL_STORAGE_H_

#include "global/common.h"
#include "global/user.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

status_t
hal_load_device_key(uint8_t* key_buf, size_t key_len);

status_t
hal_storage_get_system_state(system_state_t* out);

status_t
hal_storage_set_system_state(system_state_t* in);

// User records

status_t
hal_storage_user_get(uint8_t index, user_record_t* out);

status_t
hal_storage_user_set(uint8_t index, const user_record_t* in);

// Log records

typedef struct log_stream_t log_stream_t;

struct log_record_t;

status_t
hal_storage_log_get_size(size_t* out_size);

bool
hal_log_stream_open(log_stream_t* stream);

bool
hal_log_stream_next(log_stream_t* stream, struct log_record_t* rec);

void
hal_log_stream_close(log_stream_t* stream);

status_t
hal_storage_log_append(const uint8_t* src, size_t len);

#endif //  INCLUDE_HAL_STORAGE_H_
