#include "file.hpp"

#include <stdio.h>

File::File()
{
    m_handle = nullptr;
}

File* File::Open(const char* path, unsigned int mode)
{
    static const char* modes[] =
    {
        "r", // READ_ONLY
        "w"  // WRITE
    };

    File* file = nullptr;
    FILE* handle = nullptr;
    
    if(mode < MODE_MAX)
    {
        handle = fopen(path, modes[mode]);
        if(handle == nullptr)
        {
            printf("could not find or open file \"%s\"\n", path);
        }
    }
    else
    {
        printf("invalid file mode\n");
    }

    if(handle != nullptr)
    {
        file = new File();
        file->m_handle = handle;
    }

    return file;
}

File::~File()
{
    if(m_handle != nullptr)
    {
        fclose(m_handle);
    }
}

bool File::seek(unsigned int offset, unsigned int origin)
{
    bool status = true;
    if(fseek(m_handle, offset, origin) != 0)
    {
        status = false;
        printf("fseek failure\n");
    }
    return status;
}

long File::position()
{
    long pos = ftell(m_handle);
    if(pos == -1L)
    {
        printf("ftell failure\n");
    }
    return pos;
}

long File::size()
{
    long size = -1L;
    long curr = position();

    if(curr != -1L)
    {
        if(seek(0, SEEK_END))
        {
            size = position();
            if(size != -1L)
            {
                size = seek(curr, SEEK_SET) ? size : -1L;
            }
        }
    }

    return size;
}

bool File::read(void* buffer, unsigned int element_size, unsigned int element_count)
{
    bool status = true;
    if(fread(buffer, element_size, element_count, m_handle) != element_size * element_count)
    {
        status = false;
        printf("read failure\n");
    }
    return status;
}

int File::get_char()
{
    return fgetc(m_handle);
}

char* File::Read(const char* path, unsigned int& buffer_size)
{
    char* buffer = nullptr;
    File* file = File::Open(path, READ_ONLY);

    if(file != nullptr)
    {
        long size = file->size();

        if(size != -1L)
        {
            buffer = new char[size];

            if(!file->read(buffer, 1, size))
            {
                printf("Error: Failed to read contents of %s\n", path);
                
                delete[] buffer;
                buffer = nullptr;
            }
            else
            {
                buffer_size = size;
            }
        }

        delete file;
    }

    return buffer;
}
