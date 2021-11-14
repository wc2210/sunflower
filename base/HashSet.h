#ifndef HASHSET_H
#define HASHSET_H

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
    template <class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
    class HashSet : public Noncopyable
    {
    public:
        class Node
        {
        public:
            Node(const Key &key, Node *next) : _k(key), _next(next) {}
            void set_key(const Key &key) { _k = key; }
            void set_next(Node *next) { _next = next; }
            Key &k() { return _k; }
            Node *next() const { return _next; }

        private:
            Key _k;
            Node *_next = nullptr;
        };
        explicit HashSet(size_t power = 20);
        ~HashSet() { clear(); }

        // Capacity
        bool empty() const noexcept { return _numElements == 0; }
        size_t size() const noexcept { return _numElements; }

        // Modifiers
        std::pair<Key, bool> insert(const Key &key);
        size_t erase(const Key &key);
        void rehash(size_t capacity);
        void clear();

        // Lookup
        Key find(const Key &key);
        void find(const Key &key, bool &exsit);
        size_t count(const Key &key);

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
    template <class Key, class Hash, class KeyEqual>
    HashSet<Key, Hash, KeyEqual>::HashSet(size_t power)
    {
        _capacity = pow(2, power);
        _mask = _capacity - 1;
        _bucket[_lastest].resize(_capacity);
    }

    template <class Key, class Hash, class KeyEqual>
    size_t HashSet<Key, Hash, KeyEqual>::bucket(const Key &key) const
    {
        return POS_MOD_BASE(_hash(key));
    }

    template <class Key, class Hash, class KeyEqual>
    std::pair<Key, bool> HashSet<Key, Hash, KeyEqual>::insert(const Key &key)
    {
        size_t id = bucket(key);
        auto head = _bucket[_lastest][id];
        auto node = head;

        while (node)
        {
            if (_equal(key, node->k()))
            {
                return std::make_pair<Key, bool>(std::move(node->k()), false);
            }
            node = node->next();
        }

        Node *newNode = new Node(key, head);
        _bucket[_lastest][id] = newNode;
        _numElements++;
        auto ret = std::make_pair<Key, bool>(std::move(newNode->k()), true);

        if (_numElements >= _capacity)
        {
            rehash(next_capacity());
        }

        return ret;
    }

    template <class Key, class Hash, class KeyEqual>
    size_t HashSet<Key, Hash, KeyEqual>::erase(const Key &key)
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

    template <class Key, class Hash, class KeyEqual>
    Key HashSet<Key, Hash, KeyEqual>::find(const Key &key)
    {
        size_t id = bucket(key);
        auto node = _bucket[_lastest][id];

        while (node)
        {
            if (_equal(key, node->k()))
            {
                return node->k();
            }
            node = node->next();
        }
        return nullptr;
    }

    template <class Key, class Hash, class KeyEqual>
    void HashSet<Key, Hash, KeyEqual>::find(const Key &key, bool &exsit)
    {
        size_t id = bucket(key);
        auto node = _bucket[_lastest][id];

        while (node)
        {
            if (_equal(key, node->k()))
            {
                exsit = true;
                return;
            }
            node = node->next();
        }
        exsit = false;
    }

    template <class Key, class Hash, class KeyEqual>
    size_t HashSet<Key, Hash, KeyEqual>::count(const Key &key)
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

    template <class Key, class Hash, class KeyEqual>
    size_t HashSet<Key, Hash, KeyEqual>::next_capacity()
    {
        return 2 * _capacity;
    }

    template <class Key, class Hash, class KeyEqual>
    void HashSet<Key, Hash, KeyEqual>::rehash(size_t capacity)
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

                Node *newNode = new Node(key, _bucket[new_index][new_id]);
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
    template <class Key, class Hash, class KeyEqual>
    void HashSet<Key, Hash, KeyEqual>::clear()
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
#endif // HASHSET_H
