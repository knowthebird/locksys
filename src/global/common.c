#include "global/common.h"
#include "crypto/crypto.h"
#include <string.h>

status_t
system_state_compute_hmac(system_state_t* state)
{
    status_t status = STATUS_OK;

    if (!state)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        memset(state->hmac, 0, sizeof(state->hmac)); // Clear before computing
        size_t len = offsetof(system_state_t, hmac);
        status =
            compute_internal_hmac((const uint8_t*) state, len, state->hmac, sizeof(state->hmac));
    }

    return status;
}

status_t
system_state_validate_hmac(const system_state_t* state)
{
    status_t status                      = STATUS_OK;
    uint8_t  computed[LOCKSYS_HASH_SIZE] = {0};

    if (!state)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        size_t len = offsetof(system_state_t, hmac);
        status     = compute_internal_hmac((const uint8_t*) state, len, computed, sizeof(computed));
        if (status == STATUS_OK &&
            STATUS_OK != secure_compare(computed, state->hmac, sizeof(computed)))
        {
            status = STATUS_ERR_AUTH;
        }
    }

    return status;
}
