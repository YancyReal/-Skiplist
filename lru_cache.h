#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <list>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>

template <typename K, typename V>
class LRUCache
{
public:
    LRUCache(size_t capacity = 100, int cleanup_interval_seconds = 10)
        : _capacity(capacity),
          _cleanup_interval(cleanup_interval_seconds),
          _running(true)
    {
        _cleanup_thread = std::thread([this]()
                                      {
            while (_running.load()) {
                std::this_thread::sleep_for(std::chrono::seconds(_cleanup_interval));
                cleanup_expired();
            } });
    }

    ~LRUCache()
    {
        _running = false;
        if (_cleanup_thread.joinable())
        {
            _cleanup_thread.join();
        }
    }

    // 放入缓存（带 TTL）
    void put(const K &key, const V &value, int ttl_seconds)
    {
        auto now = std::chrono::steady_clock::now();
        auto expire_time = now + std::chrono::seconds(ttl_seconds);

        std::lock_guard<std::mutex> lock(_mutex);

        if (_map.find(key) != _map.end())
        {
            _list.erase(_map[key].second);
        }

        _list.push_front(key);
        _map[key] = {{value, expire_time}, _list.begin()};

        if (_map.size() > _capacity)
        {
            auto last = _list.back();
            _list.pop_back();
            _map.erase(last);
        }
    }

    // 获取缓存
    bool get(const K &key, V &value)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _map.find(key);
        if (it == _map.end())
            return false;

        auto now = std::chrono::steady_clock::now();
        if (now > it->second.first.expire_time)
        {
            _list.erase(it->second.second);
            _map.erase(it);
            return false;
        }

        _list.erase(it->second.second);
        _list.push_front(key);
        it->second.second = _list.begin();

        value = it->second.first.value;
        return true;
    }

    // 删除缓存
    void erase(const K &key)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _map.find(key);
        if (it != _map.end())
        {
            _list.erase(it->second.second);
            _map.erase(it);
        }
    }

    // 手动清理过期缓存项
    void cleanup_expired()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        auto now = std::chrono::steady_clock::now();
        for (auto it = _map.begin(); it != _map.end();)
        {
            if (now > it->second.first.expire_time)
            {
                _list.erase(it->second.second);
                it = _map.erase(it); // 安全地删除当前元素
            }
            else
            {
                ++it;
            }
        }
    }

private:
    struct CacheNode
    {
        V value;
        std::chrono::steady_clock::time_point expire_time;
    };

    size_t _capacity;
    int _cleanup_interval;
    std::list<K> _list;
    std::unordered_map<K, std::pair<CacheNode, typename std::list<K>::iterator>> _map;

    std::mutex _mutex;

    std::atomic<bool> _running;
    std::thread _cleanup_thread;
};

#endif