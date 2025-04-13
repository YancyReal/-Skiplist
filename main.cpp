#include <iostream>
#include <thread>
#include <chrono>
#include "skiplist.h"

int main()
{
    // 创建一个最大层数为6的跳表
    SkipList<int, std::string> skiplist(6);

    // ====== 插入数据 ======
    std::cout << "\n=== 插入元素 ===\n";
    skiplist.insert_element(1, "Apple");
    skiplist.insert_element(2, "Banana");
    skiplist.insert_element(3, "Cherry");

    // ====== 第一次查询（跳表查找 + 写入缓存） ======
    std::cout << "\n=== 第一次查询（命中跳表） ===\n";
    skiplist.search_element(1);
    skiplist.search_element(2);
    skiplist.search_element(3);

    // ====== 第二次查询（缓存命中） ======
    std::cout << "\n=== 第二次查询（命中缓存） ===\n";
    skiplist.search_element(1);
    skiplist.search_element(2);
    skiplist.search_element(3);

    // ====== 等待缓存过期（超过 TTL 5s，但未达到清理线程 20s） ======
    std::cout << "\n=== 等待缓存过期（6秒） ===\n";
    std::this_thread::sleep_for(std::chrono::seconds(6));

    // ====== 第三次查询（缓存应失效，再次命中跳表并回写缓存） ======
    std::cout << "\n=== 第三次查询（缓存失效，跳表命中） ===\n";
    skiplist.search_element(1);
    skiplist.search_element(2);
    skiplist.search_element(3);

    // ====== 删除一个元素 ======
    std::cout << "\n=== 删除元素 key=2 ===\n";
    skiplist.delete_element(2);

    // ====== 查询被删除的元素 ======
    std::cout << "\n=== 查询被删除元素 key=2（应找不到） ===\n";
    skiplist.search_element(2);

    // ====== 展示跳表结构 ======
    std::cout << "\n=== 当前跳表结构 ===\n";
    skiplist.display_list();

    // ====== Dump 到文件 ======
    std::cout << "\n=== Dump 到文件 ===\n";
    skiplist.dump_file();

    // ====== 模拟重新加载文件（创建新跳表） ======
    std::cout << "\n=== 从文件加载 ===\n";
    SkipList<int, std::string> newSkiplist(6);
    newSkiplist.load_file();

    // ====== 查询新跳表数据（验证持久化） ======
    std::cout << "\n=== 查询新跳表数据 ===\n";
    newSkiplist.search_element(1);
    newSkiplist.search_element(2); // 应该找不到（已删除）
    newSkiplist.search_element(3);

    // ====== 展示新跳表结构 ======
    std::cout << "\n=== 加载后的跳表结构 ===\n";
    newSkiplist.display_list();

    // ====== 等待 LRU 自动清除过期数据（等待25秒） ======
    std::cout << "\n=== 等待自动清理线程清除（15秒） ===\n";
    std::this_thread::sleep_for(std::chrono::seconds(15));

    // ====== 再次查询，缓存应已失效，跳表命中并重新写缓存 ======
    std::cout << "\n=== 查询缓存是否被清除（应重新写入缓存） ===\n";
    skiplist.search_element(1);
    skiplist.search_element(3);

    std::cout << "\n=== 测试完毕 ===\n";

    return 0;
}