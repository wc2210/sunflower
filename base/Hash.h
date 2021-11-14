#ifndef HASH_H
#define HASH_H

#include <string.h>

namespace sunflower
{

    /**
     * time33
     */
    size_t Time33(const char *str);

    struct CharPtrHash
    {
        size_t operator()(const char *str) const { return Time33(str); }
    };

    struct CharPtrEqual
    {
        bool operator()(const char *lhs, const char *rhs) const
        {
            return strcmp(lhs, rhs) == 0;
        }
    };

} // namespace sunflower
#endif // HASH_H
