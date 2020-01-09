#ifndef FILE_HPP
#define FILE_HPP

#include <stdio.h>

class File
{
private:
    FILE* m_handle;

private:
    File();

public:
    enum
    {
        READ_ONLY = 0,
        WRITE = 1,
        MODE_MAX = 2
    };

public:
    static File* Open(const char* path, unsigned int mode);
    static char* Read(const char* path, unsigned int& buffer_size);

public:
    ~File();

    long size();

    long position();
    bool seek(unsigned int offset, unsigned int origin);

    int get_char();
    bool read(void* buffer, unsigned int element_size, unsigned int element_count);
};

#endif // FILE_HPP