//
// Created by Xsakura on 2023/3/31.
//

#include "arena.h"

namespace massdb {

static const int kBlockSize = 4096;

Arena::Arena()
    : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}

Arena::~Arena() {
    for (char*& block : blocks_) {
        delete[] block;
    }
}

char* Arena::Allocate(size_t bytes) {
    assert(bytes > 0);
    if (bytes <= alloc_bytes_remaining_) {
        char* result = alloc_ptr_;
        alloc_ptr_ += bytes;
        alloc_bytes_remaining_ -= bytes;
        return result;
    }
    return AllocateFallback(bytes);
}

// the size of new block = bytes > kBlockSize / 4 ? bytes : kBlockSize
char* Arena::AllocateFallback(size_t bytes) {
    if (bytes > kBlockSize / 4) {
        // 如果要分配的字节大于 1024 的话新开辟一个块
        char* result = AllocateNewBlock(bytes);
        return result;
    }

    alloc_ptr_ = AllocateNewBlock(kBlockSize);
    alloc_bytes_remaining_ = kBlockSize;

    char* result = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
}

char* Arena::AllocateNewBlock(size_t block_bytes) {
    char* result = new char[block_bytes];
    blocks_.push_back(result);
    // 原子地以值和 arg 的算术加法结果替换当前值。
    // 运算是读修改写操作。按照 order 的值影响内存。
    memory_usage_.fetch_add(block_bytes + sizeof(char*),
                            std::memory_order_relaxed);
    return result;
}

char* Arena::AllocateAligned(size_t bytes) {
    // 获取当前系统的指针大小
    const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
    // (align & (align - 1)) 的含义是将变量 align 的值向
    // 下取整为 align 的最近小于等于的2的整数次幂的值，
    // 即去掉 align 低位的所有 0。这通常用于对齐内存。
    static_assert((align & (align - 1)) == 0,
                  "Pointer size should be a power of 2");
    // 下面这部分就是计算将 alloc_ptr_ 对齐 8 字节还需要多少个字节
    // (alloc_ptr_) & (align - 1) 相当于 alloc_ptr_ % 8
    size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
    // 对齐需要的字节
    size_t slop = (current_mod == 0 ? 0 : align - current_mod);
    size_t needed = bytes + slop;
    char* result;
    if (needed <= alloc_bytes_remaining_) {
        result = alloc_ptr_ + slop;
        alloc_ptr_ += needed;
        alloc_bytes_remaining_ -= needed;
    } else {
        // AllocateFallback always returned aligned memory
        result = AllocateFallback(bytes);
    }
    assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
    return result;
}

}  // namespace massdb
