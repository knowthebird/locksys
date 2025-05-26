#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>

#include "global/config.h"
#include "global/common.h"
#include "global/user.h"
#include "crypto/crypto.h"
#include "global/device_key.generated.h"
#include "hal/hal_storage.h"
#include "hal/hal_time.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <errno.h>

#ifdef USE_DPAPI_KEY
#include <wincrypt.h>
#endif


void build_executable_relative_path(char* out, const char* filename) {
    char base_path[PATH_MAX];

#ifdef _WIN32
    GetModuleFileNameA(NULL, base_path, PATH_MAX);
    char* last_slash = strrchr(base_path, '\\');
#else
    readlink("/proc/self/exe", base_path, PATH_MAX);
    char* last_slash = strrchr(base_path, '/');
#endif

    if (last_slash) *(last_slash + 1) = '\0';
    snprintf(out, PATH_MAX, "%s%s", base_path, filename);
}

void ensure_parent_dir_exists(const char* relative_filepath) {
    char full_path[PATH_MAX];
    build_executable_relative_path(full_path, relative_filepath);

    // Strip filename to isolate parent directory
    for (char* p = full_path + strlen(full_path) - 1; p > full_path; --p) {
#ifdef _WIN32
        if (*p == '\\') { *p = '\0'; break; }
#else
        if (*p == '/') { *p = '\0'; break; }
#endif
    }

#ifdef _WIN32
    CreateDirectoryA(full_path, NULL);
#else
    mkdir(full_path, 0755);
#endif
}

void generate_random_pass(char* out, size_t len) {
    if (len < 4) {
        out[0] = '\0';
        return;
    }

    const char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char digits[] = "0123456789";
    const char special[] = "!@#$^&*()";
    const char all[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$^&*()";

    out[0] = upper[rand() % (sizeof(upper) - 1)];
    out[1] = digits[rand() % (sizeof(digits) - 1)];
    out[2] = special[rand() % (sizeof(special) - 1)];

    for (size_t i = 3; i < len; ++i) {
        out[i] = all[rand() % (sizeof(all) - 1)];
    }

    for (size_t i = 0; i < len; ++i) {
        size_t j = rand() % len;
        char temp = out[i];
        out[i] = out[j];
        out[j] = temp;
    }

    out[len] = '\0';
}

#ifdef USE_DPAPI_KEY
status_t save_device_key_windows(const uint8_t* key_buf, size_t key_len) {
    ensure_parent_dir_exists(DEVICE_KEY_FILE);
    DATA_BLOB in_blob = { .pbData = (BYTE*)key_buf, .cbData = (DWORD)key_len };
    DATA_BLOB out_blob;

    if (!CryptProtectData(&in_blob, NULL, NULL, NULL, NULL, 0, &out_blob)) {
        fprintf(stderr, "CryptProtectData failed.\n");
        return STATUS_ERR_INTERNAL;
    }

    FILE* f = fopen(DEVICE_KEY_FILE, "wb");
    if (!f) {
        fprintf(stderr, "Could not open device key file for writing.\n");
        LocalFree(out_blob.pbData);
        return STATUS_ERR_INTERNAL;
    }

    fwrite(out_blob.pbData, 1, out_blob.cbData, f);
    fclose(f);
    LocalFree(out_blob.pbData);
    return STATUS_OK;
}
#endif

int main() {
    if (sizeof(DEVICE_KEY) != DEVICE_KEY_LEN) {
        fprintf(stderr, "Firmware key length mismatch\n");
        return 1;
    }
    uint8_t device_key[DEVICE_KEY_LEN];
    memcpy(device_key, DEVICE_KEY, DEVICE_KEY_LEN);

#ifdef USE_DPAPI_KEY
    if (save_device_key_windows(device_key, DEVICE_KEY_LEN) != STATUS_OK) {
        fprintf(stderr, "Failed to save device key\n");
        return 1;
    }
#endif

    srand((unsigned) time(NULL));

    char pass[CONFIG_MAX_PASSWORD_LENGTH + 1];
    generate_random_pass(pass, CONFIG_MAX_PASSWORD_LENGTH);

    printf("Generating root admin account: %s\nWith initial password:         %s\n", ROOT_ADMIN_USERNAME, pass);

    char init_path[PATH_MAX];
    build_executable_relative_path(init_path, "init_pass.txt");

    FILE* f = fopen(init_path, "w");
    if (f) {
        fprintf(f, "%s\n", pass);
        fclose(f);
        printf("Saved init password to: %s\n", init_path);
    } else {
        perror("Failed to save init_pass.txt");
    }

    ensure_parent_dir_exists(STORAGE_FILENAME);
    ensure_parent_dir_exists(LOG_STORAGE_FILENAME);

    system_state_t state = {0};
    status_t s = hal_storage_set_system_state(&state);
    if (s != STATUS_OK) {
        fprintf(stderr, "Failed to write system state to storage\n");
        return 1;
    }

    user_add(ROOT_ADMIN_USERNAME, pass, 1);

    return 0;
}
