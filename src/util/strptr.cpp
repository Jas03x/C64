#include <strptr.hpp>

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
