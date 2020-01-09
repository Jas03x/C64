#include <strptr.hpp>

strptr::strptr()
{
    this->ptr = nullptr;
    this->len = 0;
}

strptr::strptr(const char* str_ptr, unsigned int str_len)
{
    this->ptr = str_ptr;
    this->len = str_len;
}

bool strptr::operator < (const strptr& rhs) const
{
    bool ret = false;

    if(this->len < rhs.len)
    {
        ret = true;
    }
    else if(this->len == rhs.len)
    {
        for(unsigned int i = 0; i < this->len; i++)
        {
            if(this->ptr[i] < rhs.ptr[i])
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}
