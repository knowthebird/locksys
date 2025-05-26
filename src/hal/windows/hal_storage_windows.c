#include "global/config.h"

#include "hal/hal_storage.h"

#if defined(PLATFORM_WINDOWS)

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>

#include <direct.h>

#include "logging/logging.h"

#ifdef USE_FIRMWARE_KEY
#include "global/device_key.generated.h"
#endif

#ifdef USE_DPAPI_KEY
#include <wincrypt.h>
#endif

struct log_stream_t
{
    FILE* file;
};

#define RELATIVE_STORAGE_DIR "build/storage/"
#define MAX_STORAGE_SIZE 64

static void
build_executable_relative_path(char* out, const char* filename)
{
    char  base_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, base_path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        strcpy(out, filename);
        return;
    }

    char* last_slash = strrchr(base_path, '\\');
    if (last_slash)
        *(last_slash + 1) = '\0';
    snprintf(out, MAX_PATH, "%s%s", base_path, filename);
}

static void
ensure_parent_dir_exists(const char* relative_filepath)
{
    char full_path[MAX_PATH];
    build_executable_relative_path(full_path, relative_filepath);

    // Convert forward slashes to backslashes on Windows
    for (char* p = full_path; *p; ++p)
    {
        if (*p == '/')
            *p = '\\';
    }

    // Strip filename to get directory
    for (char* p = full_path + strlen(full_path) - 1; p > full_path; --p)
    {
        if (*p == '\\')
        {
            *p = '\0';
            break;
        }
    }

    // Recursively create directories
    char temp_path[MAX_PATH];
    strncpy(temp_path, full_path, MAX_PATH);
    for (char* p = temp_path + 3; *p; ++p)
    {
        if (*p == '\\')
        {
            *p = '\0';
            _mkdir(temp_path); // Ignore errors (already exists)
            *p = '\\';
        }
    }
    _mkdir(temp_path); // Final dir
}

static void
build_full_path_from_exe_dir(const char* relative_path, char* out_path, size_t out_len)
{
    char  exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        strncpy(out_path, relative_path, out_len);
        out_path[out_len - 1] = '\0';
        return;
    }

    for (int i = strnlen(exe_path, MAX_PATH) - 1; i >= 0; --i)
    {
        if (exe_path[i] == '\\' || exe_path[i] == '/')
        {
            exe_path[i + 1] = '\0';
            break;
        }
    }

    snprintf(out_path, out_len, "%s%s", exe_path, relative_path);
}

#ifdef USE_DPAPI_KEY
static const char*
get_device_key_path()
{
    static char abs_path[MAX_PATH];
    build_full_path_from_exe_dir("storage/device_key.dpapi", abs_path, sizeof(abs_path));
    return abs_path;
}
#endif

status_t
hal_load_device_key(uint8_t* key_buf, size_t key_len)
{
#ifdef USE_FIRMWARE_KEY
    if (key_len != sizeof(DEVICE_KEY))
        return STATUS_ERR_INTERNAL;
    memcpy(key_buf, DEVICE_KEY, key_len);
    return STATUS_OK;
#endif

#ifdef USE_DPAPI_KEY
    // Read encrypted blob from file
    FILE* f = fopen(get_device_key_path(), "rb");
    if (!f)
    {
        return STATUS_ERR_INTERNAL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* encrypted = malloc(size);
    if (!encrypted)
    {
        fclose(f);
        return STATUS_ERR_INTERNAL;
    }

    fread(encrypted, 1, size, f);
    fclose(f);

    DATA_BLOB in_blob  = {.pbData = encrypted, .cbData = (DWORD) size};
    DATA_BLOB out_blob = {0};

    BOOL ok = CryptUnprotectData(&in_blob, NULL, NULL, NULL, NULL, 0, &out_blob);
    free(encrypted);

    if (!ok || out_blob.cbData != key_len)
    {
        if (out_blob.pbData)
            LocalFree(out_blob.pbData);
        return STATUS_ERR_INTERNAL;
    }

    memcpy(key_buf, out_blob.pbData, key_len);
    LocalFree(out_blob.pbData);
    return STATUS_OK;
#endif
}

status_t
hal_storage_get_system_state(system_state_t* out)
{
    status_t status = STATUS_ERR_INPUT;
    FILE*    file   = NULL;

    if (out)
    {
        char abs_path[MAX_PATH];
        build_full_path_from_exe_dir(STORAGE_FILENAME, abs_path, sizeof(abs_path));
        file = fopen(abs_path, "rb");
        if (file)
        {
            if (fseek(file, CONFIG_STORAGE_INDEX_SYSTEM_STATE * sizeof(user_record_t), SEEK_SET) ==
                0)
            {
                if (fread(out, sizeof(system_state_t), 1, file) == 1)
                {
                    status = STATUS_OK;
                }
                else
                {
                    status = STATUS_ERR_STORAGE;
                }
            }
            else
            {
                status = STATUS_ERR_STORAGE;
            }
            fclose(file);
        }
        else
        {
            status = STATUS_ERR_STORAGE;
        }
    }

    if (STATUS_OK == status)
    {
        status = system_state_validate_hmac(out);
    }

    return status;
}

status_t
hal_storage_set_system_state(system_state_t* in)
{
    status_t status = STATUS_ERR_INPUT;
    FILE*    file   = NULL;

    if (in)
    {
        char abs_path[MAX_PATH];
        build_full_path_from_exe_dir(STORAGE_FILENAME, abs_path, sizeof(abs_path));

        ensure_parent_dir_exists(STORAGE_FILENAME);
        file = fopen(abs_path, "r+b");
        if (!file)
        {
            // Try to create and initialize the file
            file = fopen(abs_path, "w+b");
            if (file)
            {
                user_record_t blank = {0};
                for (int i = 0; i < CONFIG_TOTAL_STORAGE_SLOTS; ++i)
                {
                    fwrite(&blank, sizeof(user_record_t), 1, file);
                }
            }
        }

        if (file)
        {
            if (fseek(file, CONFIG_STORAGE_INDEX_SYSTEM_STATE * sizeof(user_record_t), SEEK_SET) ==
                0)
            {
                status_t hmac_status = system_state_compute_hmac(in);
                if (hmac_status != STATUS_OK)
                {
                    status = hmac_status;
                }
                else
                {
                    if (fwrite(in, sizeof(system_state_t), 1, file) == 1)
                    {
                        fflush(file);
                        _commit(_fileno(file));
                        status = STATUS_OK;
                    }
                    else
                    {
                        status = STATUS_ERR_STORAGE;
                    }
                }
            }
            else
            {
                status = STATUS_ERR_STORAGE;
            }
            fclose(file);
        }
        else
        {
            status = STATUS_ERR_STORAGE;
        }
    }

    return status;
}

status_t
hal_storage_log_append(const uint8_t* src, size_t len)
{
    if (!src || len != sizeof(log_record_t))
        return STATUS_ERR_INPUT;

    char abs_path[MAX_PATH];
    build_full_path_from_exe_dir(LOG_STORAGE_FILENAME, abs_path, sizeof(abs_path));

    FILE* file = fopen(abs_path, "ab"); // Open in append mode
    if (!file)
        return STATUS_ERR_INTERNAL;

    size_t written = fwrite(src, 1, len, file);
    fflush(file);
    _commit(_fileno(file));

    return (written == len) ? STATUS_OK : STATUS_ERR_INTERNAL;
}

bool
hal_log_stream_open(log_stream_t* stream)
{
    bool result = false;

    if (stream)
    {
        char path[MAX_PATH];
        build_full_path_from_exe_dir(LOG_STORAGE_FILENAME, path, sizeof(path));

        FILE* f = fopen(path, "rb");
        if (f)
        {
            stream->file = f;
            result       = true;
        }
        else
        {
            stream->file = NULL;
        }
    }

    return result;
}

bool
hal_log_stream_next(log_stream_t* stream, log_record_t* rec)
{
    bool result = false;

    if (stream && stream->file && rec)
    {
        uint8_t* buffer     = (uint8_t*) rec;
        size_t   total_read = 0;

        // Search for sync byte (0xA5)
        int c;
        while ((c = fgetc(stream->file)) != EOF)
        {
            if ((uint8_t) c == 0xA5)
            {
                buffer[0]  = (uint8_t) c;
                total_read = 1;
                break;
            }
        }

        // If we never found sync byte
        if (total_read == 0)
        {
            return false;
        }

        // Read the rest of the log record
        size_t remaining = sizeof(log_record_t) - 1;
        size_t read      = fread(buffer + 1, 1, remaining, stream->file);

        if (read == remaining)
        {
            result = true;
        }
    }

    return result;
}

void
hal_log_stream_close(log_stream_t* stream)
{
    if (stream)
    {
        if (stream->file)
        {
            fclose(stream->file);
            stream->file = NULL;
        }
    }
}

status_t
hal_storage_user_get(uint8_t index, user_record_t* out)
{
    char abs_path[MAX_PATH];
    build_full_path_from_exe_dir(STORAGE_FILENAME, abs_path, sizeof(abs_path));

    status_t status = STATUS_ERR_STORAGE;
    FILE*    file   = NULL;

    if (!out || index >= MAX_USERS)
        return STATUS_ERR_INPUT;

    file = fopen(abs_path, "rb");
    if (!file)
        return STATUS_ERR_STORAGE;

    if (fseek(file, index * sizeof(user_record_t), SEEK_SET) == 0)
    {
        if (fread(out, sizeof(user_record_t), 1, file) == 1)
        {
            status = STATUS_OK;
        }
    }

    fclose(file);
    return status;
}

status_t
hal_storage_user_set(uint8_t index, const user_record_t* in)
{
    char abs_path[MAX_PATH];
    build_full_path_from_exe_dir(STORAGE_FILENAME, abs_path, sizeof(abs_path));

    status_t status = STATUS_ERR_STORAGE;
    FILE*    file   = NULL;

    if (!in || index >= MAX_USERS)
    {
        return STATUS_ERR_INPUT;
    }

    ensure_parent_dir_exists(STORAGE_FILENAME);
    file = fopen(abs_path, "r+b");
    if (!file)
    {
        // Try to create it
        file = fopen(abs_path, "w+b");
        if (!file)
        {
            return STATUS_ERR_STORAGE;
        }

        // Initialize with zeros
        user_record_t blank = {0};
        for (int i = 0; i < MAX_USERS; ++i)
        {
            fwrite(&blank, sizeof(user_record_t), 1, file);
        }
    }

    if (fseek(file, index * sizeof(user_record_t), SEEK_SET) == 0)
    {
        if (fwrite(in, sizeof(user_record_t), 1, file) == 1)
        {
            status = STATUS_OK;
        }
    }

    fclose(file);
    return status;
}

status_t
hal_storage_log_get_size(size_t* out_size)
{
    if (!out_size)
    {
        return STATUS_ERR_INPUT;
    }

    FILE* file = fopen(LOG_STORAGE_FILENAME, "rb");
    if (!file)
    {
        return STATUS_ERR_STORAGE;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return STATUS_ERR_STORAGE;
    }

    long size = ftell(file);
    fclose(file);

    if (size < 0)
    {
        return STATUS_ERR_STORAGE;
    }

    *out_size = (size_t) size;
    return STATUS_OK;
}

#endif // PLATFORM_WINDOWS