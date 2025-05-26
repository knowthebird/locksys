//  Copyright 2025 Ross Kinard

#ifndef INCLUDE_HAL_IO_H_
#define INCLUDE_HAL_IO_H_

#include "global/common.h"
#include <stdbool.h>
#include <stdint.h>

status_t
hal_display(const char* msg);

status_t
hal_delay_ms(uint32_t ms);

char
hal_get_input_char_raw(void); //  For raw char-by-char input (PIN masking)
char
hal_get_input_char_line(void); //  For visible buffered input, y/n prompts

status_t
hal_flush_input_line(void); //  Discard trailing input (like \n)

status_t
hal_is_reset_jumper_enabled(bool* is_enabled);

//  🔐 Missing lock-related functions
status_t
hal_lock_open(void);
status_t
hal_lock_close(void);

#endif //  INCLUDE_HAL_IO_H_
