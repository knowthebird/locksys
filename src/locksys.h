#ifndef LOCKSYS_H
#define LOCKSYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "global/common.h"
#include "global/config.h"
#include "logging/logging.h"

status_t locksys_init();

status_t locksys_step();

status_t locksys_open_lock(const char* username, char* passphrase);

status_t locksys_close_lock();

status_t locksys_reset_passphrase(const char* username,
    char* current_passphrase, char* new_passphrase);

#ifdef __cplusplus
}
#endif


#endif //  LOCKSYS_H
