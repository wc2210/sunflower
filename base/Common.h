#ifndef COMMON_H
#define COMMON_H

namespace sunflower{

#define  POS_MOD_BASE(x) ((x) & (mask_))

#define LIKELY(x) __builtin_expect(!!(x),1)
#define UNLIKELY(x) __builtin_expect(!!(x),0)

} // namespace sunflower
#endif //COMMON_H
