#!/bin/bash

# ========== 配置参数 ==========
THREADS=4
SKIPLIST_HEIGHT=18
TEST_COUNT=100000
SRC_FILE="test_stress.cpp"
EXE_FILE="stress_test"
LOG_FILE="stress_test_log.txt"

# ========== 编译 ==========
echo "🛠️ 正在编译 $SRC_FILE..."
g++ -std=c++11 -pthread $SRC_FILE -o $EXE_FILE

if [ $? -ne 0 ]; then
    echo "❌ 编译失败"
    exit 1
fi
echo "✅ 编译成功"

# ========== 运行 ==========
echo ""
echo "🚀 启动压力测试："
echo "线程数：${THREADS}"
echo "跳表高度：${SKIPLIST_HEIGHT}"
echo "测试数据数：${TEST_COUNT}"
echo ""

# 正确传入 3 个参数：线程数、跳表高度、测试数据量
./$EXE_FILE $THREADS $SKIPLIST_HEIGHT $TEST_COUNT | tee $LOG_FILE

echo ""
echo "📄 测试完成，日志已保存到：$LOG_FILE"