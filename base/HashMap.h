#ifndef HASHMAP_H
#define HASHMAP_H

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
    class HashMap : public Noncopyable
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
        explicit HashMap(size_t power = 20);
        ~HashMap() { clear(); }

        // Capacity
        bool empty() const noexcept { return _numElements == 0; }
        size_t size() const noexcept { return _numElements; }

        // Modifiers
        std::pair<Value, bool> insert(const Key &key, const Value &value);
        size_t erase(const Key &key);
        void rehash(size_t capacity);
        void clear();

        // Lookup
        Value find(const Key &key);
        void find(const Key &key, Value &value, bool &exsit);
        size_t count(const Key &key);
        Value &operator[](const Key &key);
        Value &at(const Key &key);

        // Bucket interface
        size_t bucket_count() const { return _capacity; }
        size_t bucket(const Key &key) const;

    private:
        size_t next_capacity();

    private:
        // lastest bucket and old bucket for rehash
        std::vector<Node *> _bucket[2];
        size_t _numElements = 0;
        size_t _capacity = 0;
        size_t _mask = 0;
        size_t _lastest = 0;
        Hash _hash;
        KeyEqual _equal;
    };
    template <class Key, class Value, class Hash, class KeyEqual>
    HashMap<Key, Value, Hash, KeyEqual>::HashMap(size_t power)
    {
        _capacity = pow(2, power);
        _mask = _capacity - 1;
        _bucket[_lastest].resize(_capacity);
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    size_t HashMap<Key, Value, Hash, KeyEqual>::bucket(const Key &key) const
    {
        return POS_MOD_BASE(_hash(key));
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    std::pair<Value, bool> HashMap<Key, Value, Hash, KeyEqual>::insert(const Key &key, const Value &value)
    {
        size_t id = bucket(key);
        auto head = _bucket[_lastest][id];
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
        _bucket[_lastest][id] = newNode;
        _numElements++;
        auto ret = std::make_pair<Value, bool>(std::move(newNode->v()), true);

        if (_numElements >= _capacity)
        {
            rehash(next_capacity());
        }

        return ret;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    size_t HashMap<Key, Value, Hash, KeyEqual>::erase(const Key &key)
    {
        size_t id = bucket(key);
        auto node = _bucket[_lastest][id];
        Node *prev = nullptr;

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
                    _bucket[_lastest][id] = node->next();
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
    Value HashMap<Key, Value, Hash, KeyEqual>::find(const Key &key)
    {
        size_t id = bucket(key);
        auto node = _bucket[_lastest][id];

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
    void HashMap<Key, Value, Hash, KeyEqual>::find(const Key &key, Value &value, bool &exsit)
    {
        size_t id = bucket(key);
        auto node = _bucket[_lastest][id];

        while (node)
        {
            if (_equal(key, node->k()))
            {
                exsit = true;
                value = node->v();
                return;
            }
            node = node->next();
        }
        exsit = false;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    size_t HashMap<Key, Value, Hash, KeyEqual>::count(const Key &key)
    {
        size_t id = bucket(key);
        auto node = _bucket[_lastest][id];

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

    template <class Key, class Value, class Hash, class KeyEqual>
    Value &HashMap<Key, Value, Hash, KeyEqual>::operator[](const Key &key)
    {
        size_t id = bucket(key);
        auto head = _bucket[_lastest][id];
        auto node = head;

        while (node)
        {
            if (_equal(key, node->k()))
            {
                return node->v();
            }
            node = node->next();
        }

        Node *newNode = new Node(key, head);
        _bucket[_lastest][id] = newNode;
        _numElements++;
        auto &ret = newNode->v();
        if (_numElements >= _capacity)
        {
            rehash(next_capacity());
        }
        return ret;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    Value &HashMap<Key, Value, Hash, KeyEqual>::at(const Key &key)
    {
        size_t id = bucket(key);
        auto head = _bucket[_lastest][id];
        auto node = head;

        while (node)
        {
            if (_equal(key, node->k()))
            {
                return node->v();
            }
            node = node->next();
        }

        Node *newNode = new Node(key, head);
        _bucket[_lastest][id] = newNode;
        _numElements++;
        auto &ret = newNode->v();
        if (_numElements >= _capacity)
        {
            rehash(next_capacity());
        }
        return ret;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    size_t HashMap<Key, Value, Hash, KeyEqual>::next_capacity()
    {
        return 2 * _capacity;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    void HashMap<Key, Value, Hash, KeyEqual>::rehash(size_t capacity)
    {
        if (_capacity == capacity)
            return;

        size_t new_index = (_lastest == 0 ? 1 : 0);
        size_t new_mask = capacity - 1;

        _bucket[new_index].resize(capacity);

        // copy from _lastest bucket to new_index bucket
        for (size_t id = 0; id < _capacity; id++)
        {
            auto node = _bucket[_lastest][id];

            while (node)
            {
                auto key = node->k();
                size_t new_id = _hash(key) & new_mask;

                Node *newNode = new Node(key, node->v(), _bucket[new_index][new_id]);
                _bucket[new_index][new_id] = newNode;

                Node *prev = node;
                node = node->next();
                delete prev;
            }
            _bucket[_lastest][id] = nullptr;
        }
        _lastest = new_index;
        _mask = new_mask;
        _capacity = capacity;
    }

    template <class Key, class Value, class Hash, class KeyEqual>
    void HashMap<Key, Value, Hash, KeyEqual>::clear()
    {
        if (_numElements == 0)
            return;
        for (size_t id = 0; id < _capacity; id++)
        {
            auto node = _bucket[_lastest][id];
            while (node)
            {
                Node *curr = node;
                node = node->next();
                delete curr;
            }
            _bucket[_lastest][id] = nullptr;
        }
        _numElements = 0;
    }
} // namespace sunflower
#endif // HASHTABLE_H
