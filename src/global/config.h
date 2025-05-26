#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

// ==== Platform Detection ====

#if defined(ARDUINO) || defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_ESP32) ||                \
    defined(ARDUINO_ARCH_RP2040)
#define PLATFORM_ARDUINO

#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS

#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define PLATFORM_POSIX

#else
#error "Unknown platform: please define PLATFORM_ARDUINO, PLATFORM_WINDOWS, or PLATFORM_POSIX"
#endif

// ==== Cryptographic Backend Detection ====

#if defined(CRYPTO_BACKEND_TINYCRYPT)
#define USE_TINYCRYPT

#elif defined(CRYPTO_BACKEND_MBEDTLS)
#define USE_MBEDTLS

#elif defined(PLATFORM_ARDUINO)
// Default for Arduino builds: TinyCrypt
#define USE_TINYCRYPT

#else
#error "No crypto backend defined. Define CRYPTO_BACKEND_TINYCRYPT or CRYPTO_BACKEND_MBEDTLS."
#endif

// ==== Key Storage Option ====
#define USE_FIRMWARE_KEY
// #define USE_DPAPI_KEY        // Windows only

#ifdef USE_DPAPI_KEY
#define DEVICE_KEY_FILE "storage/device_key.dpapi"
#endif

// ==== Application Version ====
#define APP_VERSION 1

// ==== Data Storage ====
#define STORAGE_FILENAME "storage/storage.bin"

// ==== Users ====
#define ROOT_ADMIN_USERNAME "rootadmin"
#define MAX_USERS 10

// ==== Login Throttling ====
#define CONFIG_THROTTLE_DELAY_PER_FAILURE 2
#define CONFIG_THROTTLE_DELAY_MAX 30
#define CONFIG_STORAGE_INDEX_SYSTEM_STATE MAX_USERS
#define CONFIG_TOTAL_STORAGE_SLOTS (MAX_USERS + 1)

// ==== PIN and Access Control ====
#define MAX_USERNAME_LEN 32
#define CONFIG_MIN_PASSWORD_LENGTH 8
#define CONFIG_MAX_PASSWORD_LENGTH 16
#define CONFIG_REQUIRE_DIGIT 1
#define CONFIG_REQUIRE_UPPERCASE 1
#define CONFIG_REQUIRE_SYMBOL 1
#define CONFIG_PASSWORD_DELAY 1000
#define CONFIG_TIMEOUT_MS 10000
#define DEVICE_KEY_LEN 16
#define LOCKSYS_HASH_SIZE 32
#define LOCKSYS_MAX_ATTEMPTS 5

// ==== Logging ====
#define LOG_HMAC_SIZE 32
#define LOG_MAX_PAYLOAD 123
#define LOG_ENTRY_SIZE 163
#define LOG_MAX_ENTRIES 50
#define LOG_MAX_SIZE_BYTES 8150
#define LOG_STORAGE_FILENAME "storage/log.bin"

#endif // INCLUDE_CONFIG_H_
