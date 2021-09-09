#ifndef CACHE_HPP
#define CACHE_HPP

#include "cache_policy.hpp"

#include <cstddef>
#include <limits>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace caches
{

// Base class for caching algorithms
template <typename Key, typename Value, typename Policy = NoCachePolicy<Key>>
class fixed_sized_cache
{
  public:
    using iterator = typename std::unordered_map<Key, Value>::iterator;
    using const_iterator =
        typename std::unordered_map<Key, Value>::const_iterator;
    using operation_guard = typename std::lock_guard<std::mutex>;

    explicit fixed_sized_cache(size_t max_size) : max_cache_size(max_size)
    {
        if (max_cache_size == 0)
        {
            max_cache_size = std::numeric_limits<size_t>::max();
        }
    }

    ~fixed_sized_cache()
    {
        Clear();
    }

    void Put(const Key &key, const Value &value)
    {
        operation_guard lock{safe_op};
        auto elem_it = FindForChange(key);

        if (elem_it == cache_items_map.end())
        {
            // add new element to the cache
            if (cache_items_map.size() + 1 > max_cache_size)
            {
                auto disp_candidate_key = cache_policy.ReplCandidate();

                Erase(disp_candidate_key);
            }

            Insert(key, value);
        }
        else
        {
            cache_policy.Touch(key);
            elem_it->second = value;
        }
    }

    const Value &Get(const Key &key) const
    {
        operation_guard lock{safe_op};
        auto elem_it = FindElem(key);

        if (elem_it == cache_items_map.cend())
        {
            throw std::range_error{"No such element in the cache"};
        }
        cache_policy.Touch(key);

        return elem_it->second;
    }

    bool Cached(const Key &key) const
    {
        operation_guard lock{safe_op};
        return FindElem(key) != cache_items_map.end();
    }

    size_t Size() const
    {
        operation_guard lock{safe_op};
        return cache_items_map.size();
    }

    /**
     * Remove an element specified by key
     * @param key
     * @return
     * @retval true if an element specified by key was found and deleted
     * @retval false if an element is not present in a cache
     */
    bool Remove(const Key &key)
    {
        operation_guard lock{safe_op};
        if (cache_items_map.find(key) == cache_items_map.cend())
        {
            return false;
        }

        Erase(key);

        return true;
    }

  protected:
    void Clear()
    {
        operation_guard lock{safe_op};
        for (auto it = begin(); it != end(); ++it)
        {
            cache_policy.Erase(it->first);
        }
        cache_items_map.clear();
    }

    typename std::unordered_map<Key, Value>::const_iterator begin() const
    {
        return cache_items_map.cbegin();
    }

    typename std::unordered_map<Key, Value>::const_iterator end() const
    {
        return cache_items_map.cend();
    }

  protected:
    void Insert(const Key &key, const Value &value)
    {
        cache_policy.Insert(key);
        cache_items_map.emplace(std::make_pair(key, value));
    }

    void Erase(const Key &key)
    {
        cache_policy.Erase(key);

        auto elem_it = FindElem(key);
        cache_items_map.erase(elem_it);
    }

    const_iterator FindElem(const Key &key) const
    {
        return cache_items_map.find(key);
    }

    iterator FindForChange(const Key &key)
    {
        return cache_items_map.find(key);
    }

  private:
    std::unordered_map<Key, Value> cache_items_map;
    mutable Policy cache_policy;
    mutable std::mutex safe_op;
    size_t max_cache_size;
};
} // namespace caches

#endif // CACHE_HPP
