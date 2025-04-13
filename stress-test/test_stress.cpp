#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <iomanip>
#include "../skiplist.h"

int THREADS = 4;
int SKIPLIST_MAX_HEIGHT = 18;
int TEST_COUNT = 100000;
std::vector<int> test_sizes;

SkipList<int, std::string> *skipList;

// 保存测试结果
std::vector<double> insert_times;
std::vector<double> insert_qps;
std::vector<double> query_times;
std::vector<double> query_qps;

void *insertElement(void *arg)
{
    std::pair<int, int> *range = (std::pair<int, int> *)arg;
    for (int i = range->first; i < range->second; i++)
    {
        skipList->insert_element(i, "value_" + std::to_string(i));
    }
    delete range;
    pthread_exit(NULL);
}

void *getElement(void *arg)
{
    std::pair<int, int> *range = (std::pair<int, int> *)arg;
    for (int i = range->first; i < range->second; i++)
    {
        skipList->search_element(i);
    }
    delete range;
    pthread_exit(NULL);
}

void run_insert_test(int data_size)
{
    pthread_t threads[THREADS];
    int chunk = data_size / THREADS;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < THREADS; ++i)
    {
        int start_index = i * chunk;
        int end_index = (i == THREADS - 1) ? data_size : (i + 1) * chunk;
        auto *range = new std::pair<int, int>(start_index, end_index);
        pthread_create(&threads[i], NULL, insertElement, (void *)range);
    }

    for (int i = 0; i < THREADS; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    double qps = data_size / elapsed.count() / 10000.0;

    insert_times.push_back(elapsed.count());
    insert_qps.push_back(qps);

    std::cout << std::fixed << std::setprecision(5)
              << std::setw(10) << (data_size / 10000) << "\t\t"
              << elapsed.count() << std::endl;
}

void run_query_test(int data_size)
{
    pthread_t threads[THREADS];
    int chunk = data_size / THREADS;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < THREADS; ++i)
    {
        int start_index = i * chunk;
        int end_index = (i == THREADS - 1) ? data_size : (i + 1) * chunk;
        auto *range = new std::pair<int, int>(start_index, end_index);
        pthread_create(&threads[i], NULL, getElement, (void *)range);
    }

    for (int i = 0; i < THREADS; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    double qps = data_size / elapsed.count() / 10000.0;

    query_times.push_back(elapsed.count());
    query_qps.push_back(qps);

    std::cout << std::fixed << std::setprecision(5)
              << std::setw(10) << (data_size / 10000) << "\t\t"
              << elapsed.count() << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc >= 2)
        THREADS = std::atoi(argv[1]);
    if (argc >= 3)
        SKIPLIST_MAX_HEIGHT = std::atoi(argv[2]);
    if (argc >= 4)
        TEST_COUNT = std::atoi(argv[3]);

    // 动态生成测试规模（10%、50%、100%）
    test_sizes = {
        TEST_COUNT / 10,
        TEST_COUNT / 2,
        TEST_COUNT};

    std::cout << "跳表树高：" << SKIPLIST_MAX_HEIGHT << "\n"
              << std::endl;

    // ==== 插入测试 ====
    std::cout << "采用随机插入数据测试：" << std::endl;
    std::cout << "\n插入数据规模（万条）\t耗时（秒）" << std::endl;

    insert_times.clear();
    insert_qps.clear();

    for (int size : test_sizes)
    {
        skipList = new SkipList<int, std::string>(SKIPLIST_MAX_HEIGHT);
        run_insert_test(size);
        delete skipList;
    }

    // 输出插入平均 QPS
    double total_qps = 0.0;
    for (double qps : insert_qps)
        total_qps += qps;
    double avg_qps = total_qps / insert_qps.size();

    std::cout << "每秒可处理写请求数（QPS）: "
              << std::fixed << std::setprecision(2) << avg_qps << "w\n"
              << std::endl;

    // ==== 查询测试 ====
    std::cout << "取数据操作" << std::endl;
    std::cout << "\n取数据规模（万条）\t耗时（秒）" << std::endl;

    query_times.clear();
    query_qps.clear();

    for (int size : test_sizes)
    {
        skipList = new SkipList<int, std::string>(SKIPLIST_MAX_HEIGHT);
        for (int i = 0; i < size; ++i)
        {
            skipList->insert_element(i, "value_" + std::to_string(i));
        }

        run_query_test(size);
        delete skipList;
    }

    // 输出查询平均 QPS
    total_qps = 0.0;
    for (double qps : query_qps)
        total_qps += qps;
    avg_qps = total_qps / query_qps.size();

    std::cout << "每秒可处理读请求数（QPS）: "
              << std::fixed << std::setprecision(2) << avg_qps << "w\n";

    return 0;
}