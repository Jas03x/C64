
export io;

namespace io
{
    VOID print(const U8* buffer)
    {
        for(U8* it = buffer; it != null; it++)
        {
            put_char(*it);
        }
    }

    U8 get_char();

    typedef struct { VOID* handle; } FILE;

    namespace file
    {
        FILE* open(const U8* path);
        VOID  close(FILE* handle);
    }
}
