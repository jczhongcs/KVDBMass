//
// Created by Xsakura on 2023/3/31.
//

#ifndef MASSDB_UTIL_ARENA_H
#define MASSDB_UTIL_ARENA_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <vector>

namespace massdb {

// 内存分配类
class Arena {
public:
    Arena();
    ~Arena();

    Arena(const Arena&) = delete;
    Arena operator=(const Arena&) = delete;

    // 分配指定空间
    char* Allocate(size_t bytes);
    // 分配指定空间并保证内存对齐
    char* AllocateAligned(size_t bytes);

    // 返回 Arena 中总体的内存使用大小
    size_t memory_usage() const {
        return memory_usage_.load(std::memory_order_relaxed);
    }

private:
    // 如果 bytes > kBlockSize / 4 的话，单独开辟一个 bytes 大小的块
    // 否则开辟一个 kBlockSize 大小的块
    char* AllocateFallback(size_t bytes);

    // 分配一个新块，并挂载到 blocks 上
    char* AllocateNewBlock(size_t block_bytes);

    // 分配器状态

    // 指向当前内存块的指针
    char* alloc_ptr_;
    // 当前内存块的剩余空间
    size_t alloc_bytes_remaining_;

    // 存储已分配的内存块的数组
    std::vector<char*> blocks_;

    // Arena 中总体的内存使用大小
    std::atomic<size_t> memory_usage_;
};
}  // namespace massdb

#endif  // MASSDB_UTIL_ARENA_H
