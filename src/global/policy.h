#ifndef POLICY_H_
#define POLICY_H_

#include <stddef.h>
#include "global/common.h"

// Password policy
status_t
validate_password_requirements(const char* password);

// General input safety
status_t
validate_safe_string(const char* input, size_t max_len);

#endif // POLICY_H_
