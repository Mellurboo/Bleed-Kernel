#include <status.h>
#include <assert.h>

static_assert(ERROR_CODES_COUNT == 6, "Update Error Codes");
static const char* error_code_map[] = {
    [OUT_OF_MEMORY] = "Out Of Memory",
    [OUT_OF_BOUNDS] = "Out Of Bounds",
    [UNIMPLEMENTED] = "Unimplemented",
    [NAME_LIMITS] = "Name Limit Exceeded",
    [FILE_NOT_FOUND] = "File Not Found",
    [TAR_EXTRACT_FAILURE] = "Failed to Extract .tar file"
};

extern const char* status_to_string(error_code_t err){
    return error_code_map[err];
}