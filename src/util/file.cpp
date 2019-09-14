#include "file.hpp"

#include <stdio.h>

char* File::Read(const char* path, unsigned int& buffer_size)
{
    FILE* handle = fopen(path, "r");

    if(handle == nullptr)
    {
        printf("Error: File %s could not be opened for reading\n", path);
        return nullptr;
    }

    fseek(handle, 0L, SEEK_END);
    size_t size = ftell(handle);
    rewind(handle);

    char* buffer = new char[size];

    if(fread(buffer, 1, size, handle) != size)
    {
        printf("Error: Failed to read contents of %s\n", path);
        
        delete[] buffer;
        buffer = nullptr;
    }
    else
    {
        buffer_size = size;
    }

    fclose(handle);
    
    return buffer;
}
