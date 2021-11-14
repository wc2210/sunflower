#ifndef HASHMAPSAFE_H
#define HASHMAPSAFE_H

#include "Noncopyable.h"
#include <atomic>
#include <list>
#include <math.h>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string.h>
#include <vector>
#ifndef POS_MOD_BASE
#define POS_MOD_BASE(x) (x & _mask)
#endif
namespace sunflower
{
    template <class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
    class HashMapSafe : public Noncopyable
    {
    public:
        class Node
        {
        public:
            Node(const Key &key, const Value &value, Node *next) : _k(key), _v(value), _next(next) {}
            Node(const Key &key, Node *next) : _k(key), _next(next) {}
            void set(const Key &key, const Value &value)
            {
                _k = key;
                _v = value;
            }
            void set_next(Node *next) { _next = next; }
            Key k() const { return _k; }
            Value &v() { return _v; }
            Node *next() const { return _next; }

        private:
            Key _k;
            Value _v;
            Node *_next = nullptr;
        };
        explicit HashMapSafe(size_t power = 20);
        ~HashMapSafe();

        // Capacity
        bool empty() const noexcept { return _numElements == 0; }
        size_t size() const noexcept { return _numElements.load(); }

        // Modifiers
        std::pair<Value, bool> insert(const Key &key, const Value &value);
        size_t erase(const Key &key);
        void clear();

        // Lookup
        Value find(const Key &key);
        void find(const Key &key, Value &value, bool &exsit);
        size_t count(const Key &key);

        // Bucket interface
        size_t bucket_count() const { return _capacity; }
        size_t bucket(const Key &key) const;

    private:
        std::vector<Node *> _bucket;
        std::vector<std::unique_ptr<std::mutex>> _vecMutex;
        std::atomic<size_t> _numElements;
        size_t _capacity = 0;
        size_t _mask = 0;
        Hash _hash;
        KeyEqual _equal;
    };
    template <class Key, class Value, class Hash, class KeyEqual>
    HashMapSafe<Key, Value, Hash, KeyEqual>::HashMapSafe(size_t power) : _numElements(0)
    {
        _capacity = pow(2, power);
        _mask = _capacity - 1;
        _bucket.resize(_capacity);
        _vecMutex.resize(_capacity);

        for (uint32_t i = 0; i < _capacity; i++)
        {
            _vecMutex[i] = std::make_unique<std::mutex>();
        }
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    HashMapSafe<Key, Value, Hash, KeyEqual>::~HashMapSafe()
    {
        clear();
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    size_t HashMapSafe<Key, Value, Hash, KeyEqual>::bucket(const Key &key) const
    {
        return POS_MOD_BASE(_hash(key));
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    std::pair<Value, bool> HashMapSafe<Key, Value, Hash, KeyEqual>::insert(const Key &key, const Value &value)
    {
        size_t id = bucket(key);
        std::lock_guard<std::mutex> lck(*_vecMutex[id]);

        auto head = _bucket[id];
        auto node = head;

        while (node)
        {
            if (_equal(key, node->k()))
            {
                return std::make_pair<Value, bool>(std::move(node->v()), false);
            }
            node = node->next();
        }

        Node *newNode = new Node(key, value, head);
        _bucket[id] = newNode;
        _numElements++;
        return std::make_pair<Value, bool>(std::move(newNode->v()), true);
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    size_t HashMapSafe<Key, Value, Hash, KeyEqual>::erase(const Key &key)
    {
        size_t id = bucket(key);
        Node *prev = nullptr;

        std::lock_guard<std::mutex> lck(*_vecMutex[id]);

        auto node = _bucket[id];

        while (node)
        {
            if (_equal(key, node->k()))
            {
                if (prev)
                {
                    prev->set_next(node->next());
                }
                else
                {
                    _bucket[id] = node->next();
                }
                delete node;
                _numElements--;
                return 1;
            }
            prev = node;
            node = node->next();
        }
        return 0;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    void HashMapSafe<Key, Value, Hash, KeyEqual>::clear()
    {
        if (_numElements == 0)
            return;
        for (size_t id = 0; id < _capacity; id++)
        {
            std::lock_guard<std::mutex> lck(*_vecMutex[id]);
            auto node = _bucket[id];
            while (node)
            {
                Node *curr = node;
                node = node->next();
                delete curr;
            }
            _bucket[id] = nullptr;
        }
        _numElements = 0;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    Value HashMapSafe<Key, Value, Hash, KeyEqual>::find(const Key &key)
    {
        size_t id = bucket(key);
        std::lock_guard<std::mutex> lck(*_vecMutex[id]);

        auto node = _bucket[id];

        while (node)
        {
            if (_equal(key, node->k()))
            {
                return node->v();
            }
            node = node->next();
        }
        return nullptr;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    void HashMapSafe<Key, Value, Hash, KeyEqual>::find(const Key &key, Value &value, bool &exsit)
    {
        size_t id = bucket(key);
        std::lock_guard<std::mutex> lck(*_vecMutex[id]);

        auto node = _bucket[id];

        while (node)
        {
            if (_equal(key, node->k()))
            {
                exsit = true;
                // auto node->v
                value = node->v();
                return;
            }
            node = node->next();
        }
        exsit = false;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    size_t HashMapSafe<Key, Value, Hash, KeyEqual>::count(const Key &key)
    {
        size_t id = bucket(key);
        std::lock_guard<std::mutex> lck(*_vecMutex[id]);

        auto node = _bucket[id];

        while (node)
        {
            if (_equal(key, node->k()))
            {
                return 1;
            }
            node = node->next();
        }
        return 0;
    }
} // namespace sunflower
#endif // HASHMAPSAFE_H
