
export io;

namespace io
{
    void print(const U8* buffer)
    {
        for(U8* it = buffer; it != null; it++)
        {
            put_char(*it);
        }
    }

    U8 get_char();

    typedef struct { void* handle; } FILE;

    namespace file
    {
        FILE* open(const U8* path);
        void  close(FILE* handle);
    }
}
