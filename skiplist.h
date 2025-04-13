#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include "lru_cache.h" // ✅ 引入 LRU 缓存头文件

#define STORE_FILE "store/dumpFile"

// =================== 日志控制 ===================
#define SKIPLIST_VERBOSE 1 // ✅ 设置为 1 开启调试日志，0 关闭

#if SKIPLIST_VERBOSE
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif

std::mutex mtx; // mutex for critical section
std::string delimiter = ":";

// =================== Node ===================
template <typename K, typename V>
class Node
{
public:
    Node() {}
    Node(K k, V v, int);
    ~Node();

    K get_key() const;
    V get_value() const;
    void set_value(V);

    Node<K, V> **forward;
    int node_level;

private:
    K key;
    V value;
};

template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level)
{
    this->key = k;
    this->value = v;
    this->node_level = level;
    this->forward = new Node<K, V> *[level + 1];
    for (int i = 0; i <= level; i++)
    {
        forward[i] = nullptr;
    }
}

template <typename K, typename V>
Node<K, V>::~Node()
{
    delete[] forward;
}

template <typename K, typename V>
K Node<K, V>::get_key() const
{
    return key;
}

template <typename K, typename V>
V Node<K, V>::get_value() const
{
    return value;
}

template <typename K, typename V>
void Node<K, V>::set_value(V value)
{
    this->value = value;
}

// =================== SkipList ===================
template <typename K, typename V>
class SkipList
{
public:
    SkipList(int max_level);
    ~SkipList();

    int get_random_level();
    Node<K, V> *create_node(K, V, int);
    int insert_element(K, V);
    bool search_element(K);
    void delete_element(K);

    void display_list();
    void dump_file();
    void load_file();
    void clear(Node<K, V> *);
    int size();

private:
    void get_key_value_from_string(const std::string &str, std::string *key, std::string *value);
    bool is_valid_string(const std::string &str);

private:
    int _max_level;
    int _skip_list_level;
    Node<K, V> *_header;
    std::ofstream _file_writer;
    std::ifstream _file_reader;
    int _element_count;

    LRUCache<K, V> _cache = LRUCache<K, V>(100); // ✅ 缓存容量 100
};

// =================== 实现 ===================
template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level)
    : _max_level(max_level),
      _skip_list_level(0),
      _element_count(0),
      _cache(100) // ✅ 使用构造函数初始化 LRUCache
{
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

template <typename K, typename V>
SkipList<K, V>::~SkipList()
{
    if (_file_writer.is_open())
        _file_writer.close();
    if (_file_reader.is_open())
        _file_reader.close();
    if (_header->forward[0] != nullptr)
    {
        clear(_header->forward[0]);
    }
    delete (_header);
}

template <typename K, typename V>
void SkipList<K, V>::clear(Node<K, V> *cur)
{
    if (cur->forward[0] != nullptr)
    {
        clear(cur->forward[0]);
    }
    delete (cur);
}

template <typename K, typename V>
Node<K, V> *SkipList<K, V>::create_node(const K k, const V v, int level)
{
    return new Node<K, V>(k, v, level);
}

template <typename K, typename V>
int SkipList<K, V>::get_random_level()
{
    int k = 0;
    while (rand() % 2)
    {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
}

template <typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value)
{
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != nullptr && current->get_key() == key)
    {
        LOG("Key: " << key << " already exists.");
        mtx.unlock();
        return 1;
    }

    int random_level = get_random_level();
    if (random_level > _skip_list_level)
    {
        for (int i = _skip_list_level + 1; i <= random_level; i++)
        {
            update[i] = _header;
        }
        _skip_list_level = random_level;
    }

    Node<K, V> *inserted_node = create_node(key, value, random_level);
    for (int i = 0; i <= random_level; i++)
    {
        inserted_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = inserted_node;
    }

    _element_count++;
    mtx.unlock();
    return 0;
}

template <typename K, typename V>
bool SkipList<K, V>::search_element(K key)
{
    V value;

    // ✅ 先查缓存
    if (_cache.get(key, value))
    {
        LOG("Cache hit! Key: " << key << ", Value: " << value);
        return true;
    }

    // ✅ 缓存未命中，查跳表
    Node<K, V> *current = _header;
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if (current && current->get_key() == key)
    {
        value = current->get_value();
        LOG("SkipList hit! Key: " << key << ", Value: " << value);

        _cache.put(key, value, 5);
        return true;
    }

    LOG("Not Found Key: " << key);
    return false;
}

template <typename K, typename V>
void SkipList<K, V>::delete_element(K key)
{
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != nullptr && current->get_key() == key)
    {
        for (int i = 0; i <= _skip_list_level; i++)
        {
            if (update[i]->forward[i] != current)
                break;
            update[i]->forward[i] = current->forward[i];
        }

        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == nullptr)
        {
            _skip_list_level--;
        }

        LOG("Successfully deleted key " << key);
        delete current;
        _element_count--;

        _cache.erase(key);
    }
    mtx.unlock();
    return;
}

template <typename K, typename V>
void SkipList<K, V>::display_list()
{
    std::cout << "\n*****Skip List*****\n";
    for (int i = 0; i <= _skip_list_level; i++)
    {
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != nullptr)
        {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

template <typename K, typename V>
void SkipList<K, V>::dump_file()
{
    LOG("dump_file-----------------");
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];

    while (node != nullptr)
    {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        LOG(node->get_key() << ":" << node->get_value());
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return;
}

template <typename K, typename V>
void SkipList<K, V>::load_file()
{
    _file_reader.open(STORE_FILE);
    LOG("load_file-----------------");
    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty())
        {
            continue;
        }
        insert_element(stoi(*key), *value);
        LOG("key:" << *key << " value:" << *value);
    }
    delete key;
    delete value;
    _file_reader.close();
}

template <typename K, typename V>
int SkipList<K, V>::size()
{
    return _element_count;
}

template <typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string *key, std::string *value)
{
    if (!is_valid_string(str))
        return;
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template <typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str)
{
    if (str.empty())
        return false;
    if (str.find(delimiter) == std::string::npos)
        return false;
    return true;
}

#endif // SKIPLIST_H