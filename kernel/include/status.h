#ifndef STATUS_H
#define STATUS_H

typedef enum error_code {
    OUT_OF_MEMORY,
    OUT_OF_BOUNDS,
    UNIMPLEMENTED,
    NAME_LIMITS,
    FILE_NOT_FOUND,
    TAR_EXTRACT_FAILURE,

    UNEXPECTED_FILE_TYPE,
    TRUNCATED,

    ERROR_CODES_COUNT,
} error_code_t;

extern const char* status_to_string(error_code_t err);

#endif