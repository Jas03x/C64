
public module io;

namespace io
{
    VOID print(const U8* buffer);

    U8   get_char();
    VOID put_char(const U8 c);

    typedef struct { VOID* handle; } FILE;

    namespace file
    {
        FILE* open(const U8* path);
        VOID  close(FILE* handle);
    }
}
