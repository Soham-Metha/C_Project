#ifndef UTILS_FILES
#define UTILS_FILES
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum File_error File_error;

enum File_error {
    FILE_ERR_UNABLE_TO_OPEN,
    FILE_ERR_CANT_READ,
    FILE_ERR_CANT_REACH_END,
    FILE_ERR_CANT_READ_CONTENTS,
};

struct File {
    char* content;
    uint64_t size;
    bool is_valid;
    File_error error_code;
};

File file_open(const char* file_path, const char* mode)
{
    File out = { .is_valid = true };
    FILE* f  = fopen(file_path, mode);
    if (!f) {
        out.is_valid   = false;
        out.error_code = FILE_ERR_UNABLE_TO_OPEN;
        return out;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        out.is_valid   = false;
        out.error_code = FILE_ERR_CANT_READ;
        return out;
    }

    long fileSize = ftell(f);
    if (fileSize < 0) {
        out.is_valid   = false;
        out.error_code = FILE_ERR_CANT_REACH_END;
        return out;
    }

    rewind(f);
    out.size  = fileSize;

    char* buf = (char*)malloc(out.size + 1);
    assert(buf);

    if (fread(buf, 1, out.size, f) != out.size) {
        out.is_valid   = false;
        out.error_code = FILE_ERR_CANT_READ_CONTENTS;
        return out;
    }

    buf[out.size] = '\0';
    out.content   = buf;
    return out;
}

#endif