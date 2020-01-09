#ifndef STRPTR_HPP
#define STRPTR_HPP

struct strptr
{
    const char* ptr;
    unsigned int len;

    strptr();
    strptr(const char* str_ptr, unsigned int str_len);

    bool operator < (const strptr& rhs) const;
};

#endif // STRPTR_HPP