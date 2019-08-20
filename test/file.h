#ifndef FILE_H
#define FILE_H

namespace file
{
    typedef void* handle;

    handle open(const U8* path);
    void   close(handle h);
}

#endif // FILE_H