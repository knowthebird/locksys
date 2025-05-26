file(WRITE ${OUTPUT_FILE} "#ifndef INCLUDE_DEVICE_KEY_GENERATED_H_\n")
file(APPEND ${OUTPUT_FILE} "#define INCLUDE_DEVICE_KEY_GENERATED_H_\n")
file(APPEND ${OUTPUT_FILE} "#include <stdint.h>\n\n")
file(APPEND ${OUTPUT_FILE} "static const uint8_t DEVICE_KEY[16] = {\n")

# Generate 16 random bytes using CMake's string(RANDOM)
set(KEY_BYTES "")
foreach(i RANGE 1 16)
    string(RANDOM LENGTH 2 ALPHABET "0123456789ABCDEF" HEXBYTE)
    string(APPEND KEY_BYTES "0x${HEXBYTE}, ")
endforeach()

file(APPEND ${OUTPUT_FILE} "  ${KEY_BYTES}\n};\n\n")
file(APPEND ${OUTPUT_FILE} "#endif  // INCLUDE_DEVICE_KEY_GENERATED_H_\n")

message(STATUS "Generated device key at ${OUTPUT_FILE}")
