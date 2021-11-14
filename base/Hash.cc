#include "Hash.h"

namespace sunflower
{
    size_t Time33(const char *str)
    {
        unsigned int hash = 5381;
        while (*str)
        {
            hash += (hash << 5) + (*str++);
        }
        return (hash & 0x7FFFFFFF);
    }
} // namespace sunflower
