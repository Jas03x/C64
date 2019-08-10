#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>

enum SYMBOL
{
    SYMBOL_INVALID  = 0x0,
    SYMBOL_TYPE     = 0x1,
    SYMBOL_STRUCT   = 0x2,
    SYMBOL_VARIABLE = 0x3,
    SYMBOL_FUNCTION = 0x4
};

struct Symbol
{
    uint8_t type;
    void*   value;
};

#endif // SYMBOL_H