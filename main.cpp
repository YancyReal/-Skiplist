#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main()
{

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    SkipList<int, std::string> skipList(6);
    skipList.insert_element(1, "这是");
    skipList.insert_element(3, "一个");
    skipList.insert_element(7, "简单的");
    skipList.insert_element(8, "跳表项目");
    skipList.insert_element(9, "可以作为raft的起点");
    skipList.insert_element(19, "----");
    skipList.insert_element(20, "起飞！");
    skipList.insert_element(9, "重复插入节点测试");

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();

    // skipList.load_file();

    skipList.search_element(9);
    skipList.search_element(18);

    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.display_list();
}
