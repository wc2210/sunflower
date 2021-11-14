#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H
namespace sunflower
{
    class Noncopyable
    {
    public:
        Noncopyable(const Noncopyable &) = delete;
        Noncopyable &operator=(const Noncopyable &) = delete;

    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;
    };
}
#endif
