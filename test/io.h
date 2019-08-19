
public module io;

namespace io
{
    void print(const U8* buffer);

    U8   get_char();
    void put_char(const U8 c);

    typedef struct { void* handle; } FILE;

    namespace file
    {
        FILE* open(const U8* path);
        void  close(FILE* handle);
    }
}
