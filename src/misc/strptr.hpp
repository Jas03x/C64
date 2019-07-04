#ifndef STRPTR_HPP
#define STRPTR_HPP

#include <stdint.h>

struct strptr
{
    const char* ptr;
    uint16_t    len;
};

#endif // STRPTR_HPP