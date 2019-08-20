
public module file;

namespace file
{
    typedef void* handle;

    handle open(const U8* path);
    void   close(handle h);
}
