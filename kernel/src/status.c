#include <status.h>
#include <assert.h>
#include <stdio.h>
#include <drivers/serial/serial.h>
#include <ansii.h>

static_assert(ERROR_CODES_COUNT == 7, "Update Error Codes");
static const char* error_code_map[] = {
    [OUT_OF_MEMORY] = "Out Of Memory",
    [OUT_OF_BOUNDS] = "Out Of Bounds",
    [UNIMPLEMENTED] = "Unimplemented",
    [NAME_LIMITS]   = "Name Limit Exceeded",
    [FILE_NOT_FOUND]= "File Not Found",

    [TAR_EXTRACT_FAILURE]           = "Failed to Extract .tar file",

    [SERIAL_NOT_AVAILABLE] = "Serial Output is not available on this machine",
};

const char* status_to_string(error_code_t err){
    return error_code_map[err];
}

/// @brief print the error string
/// @param err error type
/// @return err
error_code_t status_print_error(error_code_t err){
    serial_printf("%s%s\n", LOG_ERROR, status_to_string(err));
    return (int)-err;
}