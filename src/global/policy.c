#include "global/policy.h"
#include "global/common.h"
#include "global/config.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

status_t
validate_password_requirements(const char* password)
{
    status_t result;
    bool     valid = true;

    if (password == NULL)
    {
        valid = false;
    }
    else
    {
        size_t len = strnlen(password, CONFIG_MAX_PASSWORD_LENGTH + 1);

        if (len < CONFIG_MIN_PASSWORD_LENGTH || len > CONFIG_MAX_PASSWORD_LENGTH)
        {
            valid = false;
        }
        else
        {
#if CONFIG_REQUIRE_DIGIT
            bool has_digit = false;
#endif
#if CONFIG_REQUIRE_UPPERCASE
            bool has_upper = false;
#endif
#if CONFIG_REQUIRE_SYMBOL
            bool has_symbol = false;
#endif

            for (size_t i = 0; i < len; i++)
            {
                char c = (unsigned char) password[i];
#if CONFIG_REQUIRE_DIGIT
                if (isdigit((unsigned char) c))
                {
                    has_digit = true;
                }
#endif
#if CONFIG_REQUIRE_UPPERCASE
                if (isupper((unsigned char) c))
                {
                    has_upper = true;
                }
#endif
#if CONFIG_REQUIRE_SYMBOL
                if (!isalnum((unsigned char) c))
                {
                    has_symbol = true;
                }
#endif
            }

#if CONFIG_REQUIRE_DIGIT
            if (!has_digit)
            {
                valid = false;
            }
#endif
#if CONFIG_REQUIRE_UPPERCASE
            if (!has_upper)
            {
                valid = false;
            }
#endif
#if CONFIG_REQUIRE_SYMBOL
            if (!has_symbol)
            {
                valid = false;
            }
#endif
        }
    }

    if (valid)
    {
        result = STATUS_OK;
    }
    else
    {
        result = STATUS_ERR_INPUT;
    }

    return result;
}

status_t
validate_safe_string(const char* input, size_t max_len)
{
    status_t result;
    bool     valid = true;

    if (NULL == input)
    {
        valid = false;
    }
    else
    {
        size_t len = strnlen(input, max_len + 1);

        if (len > max_len)
        {
            valid = false;
        }
        else
        {
            for (size_t i = 0; i < len; i++)
            {
                unsigned char c = (unsigned char) input[i];

                if (32 > c || 127 == c)
                {
                    valid = false;
                }
            }
        }
    }

    if (valid)
    {
        result = STATUS_OK;
    }
    else
    {
        result = STATUS_ERR_INPUT;
    }

    return result;
}