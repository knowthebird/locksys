#include "global/user.h"
#include "crypto/crypto.h"
#include "global/policy.h"
#include "hal/hal_storage.h"
#include "hal/hal_time.h"
#include <string.h>

status_t
user_find_by_username(const char* name, uint8_t* out_index, user_record_t* out_user)
{
    status_t status = STATUS_ERR_NOT_FOUND;
    uint8_t  i;

    if (!name || !out_index || !out_user)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        for (i = 0; i < MAX_USERS; ++i)
        {
            user_record_t temp = {0};
            if (hal_storage_user_get(i, &temp) == STATUS_OK &&
                user_record_validate_hmac(&temp) == STATUS_OK &&
                strncmp(temp.username, name, MAX_USERNAME_LEN) == 0)
            {
                *out_index = i;
                *out_user  = temp;
                status     = STATUS_OK;
                break;
            }
        }
    }

    return status;
}

status_t
user_record_compute_hmac(user_record_t* user)
{
    status_t status = STATUS_OK;

    if (!user)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        memset(user->record_hmac, 0, sizeof(user->record_hmac));
        size_t hmac_input_len = offsetof(user_record_t, record_hmac);

        status = compute_internal_hmac((const uint8_t*) user, hmac_input_len, user->record_hmac,
                                       LOCKSYS_HASH_SIZE);
    }

    return status;
}

status_t
user_record_validate_hmac(const user_record_t* user)
{
    status_t status                           = STATUS_OK;
    uint8_t  computed_hmac[LOCKSYS_HASH_SIZE] = {0};

    if (!user)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        size_t hmac_input_len = offsetof(user_record_t, record_hmac);

        status = compute_internal_hmac((const uint8_t*) user, hmac_input_len, computed_hmac,
                                       LOCKSYS_HASH_SIZE);

        if (status == STATUS_OK)
        {
            if (STATUS_OK != secure_compare(computed_hmac, user->record_hmac, LOCKSYS_HASH_SIZE))
            {
                status = STATUS_ERR_AUTH;
            }
        }
    }

    return status;
}

status_t
user_add(const char* username, const char* password, uint8_t is_admin)
{
    // validate inputs
    if (validate_safe_string(username, MAX_USERNAME_LEN) != STATUS_OK)
        return STATUS_ERR_INPUT;
    if (validate_safe_string(password, CONFIG_MAX_PASSWORD_LENGTH) != STATUS_OK)
        return STATUS_ERR_INPUT;
    if (validate_password_requirements(password) != STATUS_OK)
        return STATUS_ERR_INPUT;

    user_record_t new_user = {0};
    strncpy(new_user.username, username, MAX_USERNAME_LEN);
    new_user.user_flags = USER_FLAG_IS_ENABLED;

    if (is_admin)
    {
        new_user.user_flags = new_user.user_flags | USER_FLAG_IS_ADMIN;
    }

    uint32_t now                         = hal_get_timestamp();
    new_user.created_timestamp           = now;
    new_user.password_last_set           = now;
    new_user.last_attempt_timestamp      = 0;
    new_user.failed_attempts_since_login = 0;

    // Compute password HMAC
    if (compute_internal_hmac((const uint8_t*) password,
                              strnlen(password, CONFIG_MAX_PASSWORD_LENGTH + 1),
                              new_user.password_hmac, LOCKSYS_HASH_SIZE) != STATUS_OK)
    {
        return STATUS_ERR_INTERNAL;
    }

    // Compute full record HMAC
    if (user_record_compute_hmac(&new_user) != STATUS_OK)
    {
        return STATUS_ERR_INTERNAL;
    }

    system_state_t state = {0};
    hal_storage_get_system_state(&state);
    uint8_t new_usr_idx = state.user_count;
    state.user_count++;
    hal_storage_set_system_state(&state);

    // Write to user slot
    if (hal_storage_user_set(new_usr_idx, &new_user) != STATUS_OK)
    {
        return STATUS_ERR_INTERNAL;
    }

    return STATUS_OK;
}

status_t
user_get_is_admin(const char* username, bool* out_is_admin)
{
    status_t status;
    bool     result = false;

    if (username == NULL || out_is_admin == NULL)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        user_record_t user;
        uint8_t       index;
        status = user_find_by_username(username, &index, &user);

        if (status == STATUS_OK)
        {
            if ((user.user_flags & USER_FLAG_IS_ADMIN) != 0)
            {
                result = true;
            }
        }
    }

    *out_is_admin = result;

    return status;
}