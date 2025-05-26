#include "locksys.h"

#include "crypto/crypto.h"
#include "global/config.h"
#include "global/policy.h"
#include "hal/hal_io.h"
#include "hal/hal_storage.h"
#include "hal/hal_time.h"
#include <string.h>

// --- Internal (static) function declarations ---
static status_t
locksys_check_passphrase(const char* username, char* passphrase);
static status_t
locksys_set_failed_attempts(const char* username, uint8_t count);
static status_t
locksys_get_failed_attempts(const char* username, uint8_t* out);
static status_t
locksys_set_permanently_locked(const char* username, bool value);
static status_t
locksys_get_permanently_locked(const char* username, bool* out);
static status_t
locksys_set_hash(const char* username, const uint8_t* hash);
static status_t
locksys_get_hash(const char* username, uint8_t* out);
static status_t
throttle_check_and_register_attempt(void);
static status_t
throttle_reset(void);

status_t
locksys_init(void)
{
    status_t      status  = STATUS_OK;
    uint8_t       version = APP_VERSION;
    user_record_t admin   = {0};
    uint8_t       index   = 0;

    log_init();
    log_write(EVENT_APPLICATION_START, &version, sizeof(version));
    // log_dump();

    status = user_find_by_username(ROOT_ADMIN_USERNAME, &index, &admin);
    if (status != STATUS_OK || user_record_validate_hmac(&admin) != STATUS_OK)
    {
        status = STATUS_ERR_TAMPER;
    }

    return status;
}

static status_t
locksys_check_passphrase(const char* username, char* passphrase)
{
    status_t rtn_status     = STATUS_OK;
    bool     is_perm_locked = false;

    status_t status = status = locksys_get_permanently_locked(username, &is_perm_locked);

    if (STATUS_OK != status)
    {
        secure_zero(passphrase, strnlen(passphrase, CONFIG_MAX_PASSWORD_LENGTH + 1));
        rtn_status = status;
    }
    else if (is_perm_locked)
    {
        secure_zero(passphrase, strnlen(passphrase, CONFIG_MAX_PASSWORD_LENGTH + 1));
        rtn_status = STATUS_ERR_PERM_LOCKED;
    }
    else
    {
        uint8_t entered_hash[LOCKSYS_HASH_SIZE];
        compute_internal_hmac((const uint8_t*) passphrase,
                              strnlen(passphrase, CONFIG_MAX_PASSWORD_LENGTH + 1), entered_hash,
                              sizeof(entered_hash));
        secure_zero(passphrase, strnlen(passphrase, CONFIG_MAX_PASSWORD_LENGTH + 1));

        uint8_t hash[LOCKSYS_HASH_SIZE];
        locksys_get_hash(username, hash);
        status = secure_compare(entered_hash, hash, LOCKSYS_HASH_SIZE);
        secure_zero(hash, sizeof(hash));
        secure_zero(entered_hash, sizeof(entered_hash));
        if (STATUS_OK == status)
        {
            locksys_set_failed_attempts(username, 0);
            throttle_reset();
            rtn_status = STATUS_OK;
        }
        else
        {
            uint8_t failed_attempts = 0;
            locksys_get_failed_attempts(username, &failed_attempts);
            locksys_set_failed_attempts(username, ++failed_attempts);

            rtn_status = STATUS_ERR_AUTH;

            locksys_get_failed_attempts(username, &failed_attempts);
            if (failed_attempts >= LOCKSYS_MAX_ATTEMPTS)
            {
                locksys_set_permanently_locked(username, true);
                rtn_status = STATUS_ERR_PERM_LOCKED;
            }
        }
    }
    return rtn_status;
}

status_t
locksys_reset_passphrase(const char* username, char* current_passphrase, char* new_passphrase)
{
    status_t status = throttle_check_and_register_attempt();
    if (status != STATUS_OK)
    {
        secure_zero(current_passphrase, CONFIG_MAX_PASSWORD_LENGTH);
        secure_zero(new_passphrase, CONFIG_MAX_PASSWORD_LENGTH);
        return status;
    }

    status = validate_safe_string(username, MAX_USERNAME_LEN);
    if (status != STATUS_OK)
    {
        return status;
    }

    status = validate_safe_string(current_passphrase, CONFIG_MAX_PASSWORD_LENGTH);
    if (status != STATUS_OK)
    {
        return status;
    }

    status = validate_safe_string(new_passphrase, CONFIG_MAX_PASSWORD_LENGTH);
    if (status != STATUS_OK)
    {
        return status;
    }

    status = validate_password_requirements(new_passphrase);
    if (status != STATUS_OK)
    {
        return status;
    }

    log_write(EVENT_REQUEST_TO_UNLOCK, 0, 0);

    status = locksys_check_passphrase(username, current_passphrase);
    secure_zero(current_passphrase,
                strnlen(current_passphrase,
                        CONFIG_MAX_PASSWORD_LENGTH + 1)); // Always clear sensitive input

    if (status == STATUS_OK)
    {
        uint8_t new_hash[LOCKSYS_HASH_SIZE];

        status = compute_internal_hmac((const uint8_t*) new_passphrase,
                                       strnlen(new_passphrase, CONFIG_MAX_PASSWORD_LENGTH + 1),
                                       new_hash, sizeof(new_hash));
        secure_zero(new_passphrase, strnlen(new_passphrase, CONFIG_MAX_PASSWORD_LENGTH + 1));

        if (status == STATUS_OK)
        {
            status = locksys_set_hash(username, new_hash);
            secure_zero(new_hash, sizeof(new_hash));
        }

        if (status == STATUS_OK)
        {
            status = locksys_set_failed_attempts(username, 0);
        }

        if (status == STATUS_OK)
        {
            log_write(EVENT_PASS_CHANGE_PASSED, (const uint8_t*) &status, sizeof(status));
        }
        else
        {
            log_write(EVENT_PASS_CHANGE_FAILED, (const uint8_t*) &status, sizeof(status));
        }
    }
    else
    {
        secure_zero(new_passphrase,
                    strnlen(new_passphrase,
                            CONFIG_MAX_PASSWORD_LENGTH + 1)); // clear on failure too
        log_write(EVENT_PASS_CHANGE_FAILED, (const uint8_t*) &status, sizeof(status));
    }

    return status;
}

status_t
locksys_open_lock(const char* username, char* passphrase)
{
    status_t status = throttle_check_and_register_attempt();
    if (status != STATUS_OK)
    {
        secure_zero(passphrase, CONFIG_MAX_PASSWORD_LENGTH);
        return status;
    }

    status = validate_safe_string(username, MAX_USERNAME_LEN);
    if (status != STATUS_OK)
    {
        return status;
    }

    status = validate_safe_string(passphrase, CONFIG_MAX_PASSWORD_LENGTH);
    if (status != STATUS_OK)
    {
        return status;
    }

    // log_dump();
    log_write(EVENT_REQUEST_TO_UNLOCK, 0, 0);

    status = locksys_check_passphrase(username, passphrase);
    if (STATUS_OK == status)
    {
        log_write(EVENT_UNLOCKING_DEVICE, 0, 0);
        status = hal_lock_open();
    }
    else
    {
        log_write(EVENT_UNLOCK_CHECK_FAILED, (const uint8_t*) &status, sizeof(status));
    }
    return status;
}

status_t
locksys_close_lock()
{
    log_write(EVENT_LOCKING_DEVICE, 0, 0);
    return hal_lock_close();
}

status_t
locksys_step()
{
    //  Optional: for future timing logic like temporary lockouts
    return STATUS_OK;
}

static status_t
locksys_set_failed_attempts(const char* username, uint8_t count)
{
    status_t      status = STATUS_OK;
    user_record_t user   = {0};
    uint8_t       index  = 0;

    if (!username)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        status = user_find_by_username(username, &index, &user);
        if (status == STATUS_OK)
        {
            user.failed_attempts_since_login = count;
            user_record_compute_hmac(&user);
            status = hal_storage_user_set(index, &user);
        }
    }

    return status;
}

static status_t
locksys_get_failed_attempts(const char* username, uint8_t* out)
{
    status_t      status = STATUS_OK;
    user_record_t user   = {0};
    uint8_t       index  = 0;

    if (!username || !out)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        status = user_find_by_username(username, &index, &user);
        if (status == STATUS_OK)
        {
            *out = user.failed_attempts_since_login;
        }
    }

    return status;
}

static status_t
locksys_set_permanently_locked(const char* username, bool locked)
{
    status_t      status = STATUS_OK;
    user_record_t user   = {0};
    uint8_t       index  = 0;

    if (!username)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        status = user_find_by_username(username, &index, &user);
        if (status == STATUS_OK)
        {
            if (locked)
            {
                user.user_flags |= USER_FLAG_IS_LOCKED;
            }
            else
            {
                user.user_flags &= ~USER_FLAG_IS_LOCKED;
            }
            user_record_compute_hmac(&user);
            status = hal_storage_user_set(index, &user);
        }
    }

    return status;
}

static status_t
locksys_get_permanently_locked(const char* username, bool* out)
{
    status_t      status = STATUS_OK;
    user_record_t user   = {0};
    uint8_t       index  = 0;

    if (!username || !out)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        status = user_find_by_username(username, &index, &user);
        if (status == STATUS_OK)
        {
            *out = (user.user_flags & USER_FLAG_IS_LOCKED) != 0;
        }
    }

    return status;
}

static status_t
locksys_set_hash(const char* username, const uint8_t* hash)
{
    status_t      status = STATUS_OK;
    user_record_t user   = {0};
    uint8_t       index  = 0;

    if (!username || !hash)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        status = user_find_by_username(username, &index, &user);
        if (status == STATUS_OK)
        {
            memcpy(user.password_hmac, hash, LOCKSYS_HASH_SIZE);
            user.password_last_set = hal_get_timestamp();
            user_record_compute_hmac(&user);
            status = hal_storage_user_set(index, &user);
        }
    }

    return status;
}

static status_t
locksys_get_hash(const char* username, uint8_t* out)
{
    status_t      status = STATUS_OK;
    user_record_t user   = {0};
    uint8_t       index  = 0;

    if (!username || !out)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        status = user_find_by_username(username, &index, &user);
        if (status == STATUS_OK)
        {
            memcpy(out, user.password_hmac, LOCKSYS_HASH_SIZE);
        }
    }

    return status;
}

static status_t
throttle_check_and_register_attempt(void)
{
    status_t       status = STATUS_OK;
    system_state_t state  = {0};
    uint32_t       now    = hal_get_timestamp();

    if (hal_storage_get_system_state(&state) == STATUS_OK)
    {
        uint32_t delay = CONFIG_THROTTLE_DELAY_PER_FAILURE * state.failed_attempts;
        if (delay > CONFIG_THROTTLE_DELAY_MAX)
        {
            delay = CONFIG_THROTTLE_DELAY_MAX;
        }

        if (state.failed_attempts > 0 && (now - state.last_attempt_time) < delay)
        {
            // Do not count attempts while throttled to defend against DOS attack
            status = STATUS_ERR_THROTTLED;
        }
        else
        {
            state.failed_attempts++;
            state.last_attempt_time = now;
            hal_storage_set_system_state(&state);
        }
    }
    else
    {
        status = STATUS_ERR_STORAGE;
    }

    return status;
}

static status_t
throttle_reset(void)
{
    status_t       status = STATUS_ERR_STORAGE;
    system_state_t state  = {0};

    if (hal_storage_get_system_state(&state) == STATUS_OK)
    {
        state.failed_attempts   = 0;
        state.last_attempt_time = hal_get_timestamp();
        status                  = hal_storage_set_system_state(&state);
    }

    return status;
}
