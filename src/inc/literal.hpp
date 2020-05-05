#ifndef LITERAL_HPP
#define LITERAL_HPP

#include <stdint.h>

enum LITERAL
{
    LITERAL_INVALID = 0x0,
    LITERAL_INTEGER = 0x1,
    LITERAL_FLOAT   = 0x2,
    LITERAL_CHAR    = 0x3,
    LITERAL_STRING  = 0x4
};

struct Literal
{
    uint8_t type;

    union
    {
        uint64_t integer_value;
        double float_value;
        char character;

        struct
        {
            const char*  ptr;
            unsigned int len;
        } string;
    } data;
};

#endif // LITERAL_HPP