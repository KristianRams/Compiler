#include "Common.h"

#if defined(WIN32)
// Returns the size of the file.
s64 read_file(char *file_name, void **data_return) { 
    HANDLE file_handle = CreateFile(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) { return -1; }

    DWORD file_size = GetFileSize(file_handle, NULL); 
    if (file_size == INVALID_FILE_SIZE) { return -1; }
    
    // +1 for nul termination
    u8 *data = new u8[file_size + 1];

    OVERLAPPED over_lapped = {};

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfileex
    // ReadFileEx documentation states it returns a Bool but it docs says the return value is in terms of  
    // either 0 for failure or non-zero for success. Sigh!!!
    BOOL success = ReadFileEx(file_handle, data, file_size, &over_lapped, NULL);
    if (success == FALSE) { delete[] data; return -1; }

    // ReadFileEx does not nul terminate the buffer.
    data[file_size] = '\0';
    *data_return = data; 

    return file_size;
}

#else  // Linux
#include <stdio.h>
#include <sys/stat.h>

s64 read_file(char *file_name, void **data_return) {
    FILE *file = fopen(file_name, "rb");
    if (!file) { return -1; }

    s32 descriptor = fileno(file);

    struct stat file_stats;
    s32 result = fstat(descriptor, &file_stats);

    if (result == -1) { return -1; }

    s32 length = file_stats.st_size;

    // +1 for nul termination
    u8 *data = new u8[length + 1];

    fseek(file, 0, SEEK_SET);
    s32 success = fread((void *)data, length, 1, file);
    if (success < 1) { delete[] data; return -1; }

    data[length] = '\0';
    *data_return = data;

    return length;
}
#endif 
