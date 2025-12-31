#pragma once

typedef enum error_code {
    OUT_OF_MEMORY,
    OUT_OF_BOUNDS,
    UNIMPLEMENTED,
    NAME_LIMITS,
    FILE_NOT_FOUND,
    SHORTREAD,
    TAR_EXTRACT_FAILURE,

    SERIAL_NOT_AVAILABLE,

    INVALID_MAGIC,

    DEV_EXISTS,
    MAX_DEVICES_REACHED,

    ERROR_CODES_COUNT,
} error_code_t;

const char* status_to_string(error_code_t err);

/// @brief print the error string
/// @param err error type
/// @return err
error_code_t status_print_error(error_code_t err);