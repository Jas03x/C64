#ifndef IO_H
#define IO_H

namespace io
{
    U8   get_char();
    void put_char(const U8 c);

    void print(const U8* buffer);
}

#endif // IO_H