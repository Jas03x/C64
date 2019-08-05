#ifndef LITERAL_HPP
#define LITERAL_HPP

#include <stdint.h>

#include <strptr.hpp>

enum LITERAL
{
    LITERAL_INVALID = 0x0,
    LITERAL_INTEGER = 0x1,
    LITERAL_DECIMAL = 0x2,
    LITERAL_STRING  = 0x3
};

struct Literal
{
    uint8_t type;

    union
    {
        struct
        {
            uint64_t value;
        } integer;

        struct
        {
            double value;
        } decimal;

        strptr string;
    };
};

#endif // LITERAL_HPP