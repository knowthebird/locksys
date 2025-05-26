cmake_minimum_required(VERSION 3.16)
project(accesscontrol C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Disable compiler optimizations globally
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O0")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O0 -g")
set(CMAKE_C_FLAGS_MINSIZEREL "-O0")

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# === Configurable Options ===
option(CRYPTO_BACKEND_TINYCRYPT "Use TinyCrypt as the crypto backend" OFF)

# === Paths ===
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)
set(GLOBAL_DIR ${SRC_DIR}/global)
set(DEVICE_KEY_HEADER ${GLOBAL_DIR}/device_key.generated.h)

include_directories(${SRC_DIR})  # Ensure headers like "crypto/crypto.h" resolve

# === Device Key Generation ===
set(DEVICE_KEY_STAMP ${CMAKE_BINARY_DIR}/device_key.stamp)

add_custom_command(
    OUTPUT ${DEVICE_KEY_STAMP}
    COMMAND ${CMAKE_COMMAND}
        -DOUTPUT_FILE=${DEVICE_KEY_HEADER}
        -P ${CMAKE_SOURCE_DIR}/tools/generate_device_key.cmake
    COMMAND ${CMAKE_COMMAND} -E touch ${DEVICE_KEY_STAMP}
    COMMENT "Generating device key header"
    VERBATIM
)

add_custom_target(generate_device_key DEPENDS ${DEVICE_KEY_STAMP})

# === Core Sources ===
file(GLOB_RECURSE CORE_SRC
    ${SRC_DIR}/crypto/*.c
    ${SRC_DIR}/global/*.c
    ${SRC_DIR}/logging/*.c
    ${SRC_DIR}/*.c
)

file(GLOB_RECURSE CORE_HEADERS
    ${SRC_DIR}/**/*.h
)

# === Filter out extern sources from tidy ===
set(LOCAL_SRC ${CORE_SRC})
list(FILTER LOCAL_SRC EXCLUDE REGEX "/extern/")

# === Select Crypto Backend Sources ===
if(CRYPTO_BACKEND_TINYCRYPT)
    message(STATUS "Using TinyCrypt backend")
    set(CRYPTO_BACKEND_SOURCES
        ${SRC_DIR}/extern/tinycrypt/source/hmac.c
        ${SRC_DIR}/extern/tinycrypt/source/sha256.c
        ${SRC_DIR}/extern/tinycrypt/source/utils.c
    )
    include_directories(${SRC_DIR}/extern/tinycrypt/include)
    add_compile_definitions(CRYPTO_BACKEND_TINYCRYPT)
else()
    message(STATUS "Using mbedTLS backend")
    set(CRYPTO_BACKEND_SOURCES
        ${SRC_DIR}/extern/mbedtls/library/md.c
        ${SRC_DIR}/extern/mbedtls/library/pkcs5.c
        ${SRC_DIR}/extern/mbedtls/library/platform.c
        ${SRC_DIR}/extern/mbedtls/library/platform_util.c
        ${SRC_DIR}/extern/mbedtls/library/sha256.c
    )
    include_directories(${SRC_DIR}/extern/mbedtls/include)
    add_compile_definitions(CRYPTO_BACKEND_MBEDTLS)
endif()

# === Platform-Specific HAL Sources ===
set(HAL_POSIX
    ${SRC_DIR}/hal/posix/hal_io_posix.c
    ${SRC_DIR}/hal/posix/hal_storage_posix.c
    ${SRC_DIR}/hal/posix/hal_time_posix.c
)

set(HAL_WINDOWS
    ${SRC_DIR}/hal/windows/hal_io_windows.c
    ${SRC_DIR}/hal/windows/hal_storage_windows.c
    ${SRC_DIR}/hal/windows/hal_time_windows.c
)

# === Main Executables ===
add_executable(main_win
    ${CORE_SRC}
    ${CRYPTO_BACKEND_SOURCES}
    ${HAL_WINDOWS}
    ${EXAMPLES_DIR}/main.c
)
set_target_properties(main_win PROPERTIES OUTPUT_NAME main)
add_dependencies(main_win generate_device_key)

add_executable(main_posix
    ${CORE_SRC}
    ${CRYPTO_BACKEND_SOURCES}
    ${HAL_POSIX}
    ${EXAMPLES_DIR}/main.c
)
set_target_properties(main_posix PROPERTIES OUTPUT_NAME main)
add_dependencies(main_posix generate_device_key)

# === Bootstrap Executables ===
add_executable(bootstrap_win
    ${CORE_SRC}
    ${CRYPTO_BACKEND_SOURCES}
    ${HAL_WINDOWS}
    ${TOOLS_DIR}/bootstrap_system.c
)
set_target_properties(bootstrap_win PROPERTIES OUTPUT_NAME bootstrap)
add_dependencies(bootstrap_win generate_device_key)

add_executable(bootstrap_posix
    ${CORE_SRC}
    ${CRYPTO_BACKEND_SOURCES}
    ${HAL_POSIX}
    ${TOOLS_DIR}/bootstrap_system.c
)
set_target_properties(bootstrap_posix PROPERTIES OUTPUT_NAME bootstrap)
add_dependencies(bootstrap_posix generate_device_key)

# === Formatting ===
find_program(CLANG_FORMAT_EXE NAMES clang-format)

# Filter out extern sources from format
set(LOCAL_HEADERS ${CORE_HEADERS})
list(FILTER LOCAL_HEADERS EXCLUDE REGEX "/extern/")
set(LOCAL_FORMAT_SRC ${CORE_SRC})
list(FILTER LOCAL_FORMAT_SRC EXCLUDE REGEX "/extern/")

add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXE} -i ${LOCAL_FORMAT_SRC} ${LOCAL_HEADERS}
    COMMENT "Formatting source code (excluding extern)"
)

# === Linting ===
add_custom_target(tidy
    COMMAND ${CLANG_TIDY_EXE}
        -p=${CMAKE_BINARY_DIR}
        -quiet
        -extra-arg=-fno-color-diagnostics
        ${LOCAL_SRC}
    COMMENT "Running clang-tidy"
    USES_TERMINAL
)


# === Unit Test Executable ===
file(GLOB TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/test_*.c
)

add_executable(unit_tests
    ${TEST_SOURCES}
)

target_include_directories(unit_tests PRIVATE
    ${SRC_DIR}
)

add_custom_target(tests_run
    COMMAND unit_tests
    DEPENDS unit_tests
    COMMENT "Running unit tests"
)


# === Coverage Placeholder ===
add_custom_target(coverage
    COMMAND echo "Coverage report generation not yet implemented"
)

# === Distclean ===
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E echo "Removing build outputs and generated key..."
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeFiles/generate_device_key.dir/device_key.stamp
    COMMENT "Completely removing build artifacts (except README.md)"
)

