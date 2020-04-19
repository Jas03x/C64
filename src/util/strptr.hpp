#ifndef STRPTR_HPP
#define STRPTR_HPP

struct strptr
{
    const char* ptr;
    unsigned int len;

    bool operator < (const strptr& rhs) const;
};

#endif // STRPTR_HPP
