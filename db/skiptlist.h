//
// Created by Xsakura on 2023/3/31.
//

#ifndef MASSDB_SKIPTLIST_H
#define MASSDB_SKIPTLIST_H

#include <atomic>
#include <cassert>

#include "util/arena.h"
#include "util/random.h"

namespace massdb {

template <typename Key, typename Comparator>
class SkipList {
private:
    struct Node;  // SkipList 中的节点

public:
    // 创建一个 SkipList 使用 "cmp" 比较 keys，并且使用 "arena" 分配内存。
    explicit SkipList(Comparator cmp, Arena* arena);

    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

    // 将关键字插入列表中。
    // 要求：当前 list 中没有与关键字相等的任何内容。
    void Insert(const Key& key);
    // 当且仅当 list 中存在 key 相同的条目（entry)
    bool Contians(const Key& key) const;

private:
    // 获取当前 SkipList 的最大高度
    inline int GetMaxHeight() const {
        return max_height_.load(std::memory_order_relaxed);
    }

    // 随机生成高度。默认最大为 12
    int RandomHeight();
    // 当前节点的值小于等于 key 返回 true
    bool KeyIsAfterNode(const Key& key, Node* n) const;
    // 当两个键相等时，返回 true
    bool Equal(const Key& a, const Key& b) const {
        return (compare_(a, b) == 0);
    }

    // 在 Arena 的基础上新建一个节点
    Node* NewNode(const Key& key, int height);
    // 将每个 list 中大于等于 key 的前一个节点记录在 prev 中
    // 并返回 level 0 中第一个大于等于 key 的节点
    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;
    // 在 SkipList 中找第一个大于等于 key 的节点
    Node* FindLessThan(const Key& key) const;
    // 找 SkipList 中最后一个元素
    Node* FindLast() const;

private:
    enum { kMaxHeight = 12 };  // level 最大高度

    Node* const head_;          // SkipList 的空头节点
    Comparator const compare_;  // 比较类
    Arena* const arena_;        // 内存分配器类

    Random rnd_;  // 随机生成器类

    std::atomic<int> max_height_;  // 当前 SkipList 的高度
};

// 跳表节点类型
template <typename Key, typename Comparator>
struct SkipList<Key, Comparator>::Node {
    explicit Node(const Key& key) : key(key) {}

    // links 的存取器和修改器。

    // 将必要的屏障（barriers）包装到方法中

    // 从跳表节点中取值
    Node* Next(int n) {
        assert(n >= 0);
        // 为了观察到完全初始化的 node，请使用 "acquire load"。
        //
        // std::memory_order_acquire 用于保证一种原子的或更新的
        // 操作读取到的变量的是其最新值，而不是缓存中的旧值。
        // 这样可以避免线程间的数据竞争和其他并发问题
        return next_[n].load(std::memory_order_acquire);
    }
    // 向跳表的第 n 层，赋值 x
    void SetNext(int n, Node* x) {
        assert(n >= 0);
        // 使用 "release store"，这样通过这个指针读取的
        // 任何人都会观察到插入 node 的完全初始化版本。
        //
        // std::memory_order_release 是 C++11 标准的一个同步内存序，
        // 用于在多线程场景下确保对内存的写入操作是同步的，
        // 也就是当使用该内存序时，所有之后的读取操作将在写入操作之后执行。
        // 这可以防止指令乱序优化或缓存的过度使用等因素带来的写入数据不一致问题。
        //
        // 当写入操作使用 std::memory_order_release 标记时，
        // 其他线程使用 std::memory_order_acquire 标记来读取该变量时，
        // 都将看到最新的值。
        next_[n].store(x, std::memory_order_release);
    }

    // 可以在少数地方安全使用的无屏障变量（No-barrier variants）

    Node* NoBarrier_Next(int n) {
        assert(n >= 0);
        // 在使用 std::memory_order_relaxed 的情况下，编译器和
        // 硬件都不需要对内存访问进行任何同步操作，因此读写操作可以乱序执行，
        // 可以在不牺牲正确性和线程安全性的前提下提高代码的性能，
        // 可能会导致读写操作的结果不一致。
        return next_[n].load(std::memory_order_relaxed);
    }
    void NoBarrier_SetNext(int n, Node* x) {
        assert(n >= 0);
        // 当确保没有其他线程读的情况可以始终这个方法存储，
        // 可以将内存访问的开销降到最低，提高程序的性能。
        // 如果该操作后面还有其他与该存储操作相关的读操作，
        // 那么这些读操作将无法读到最新的存储值。
        next_[n].store(x, std::memory_order_relaxed);
    }

    Key const key;

private:
    // next_ 数组的长度即是跳表节点的高度。
    // next_[0] 就是 level 0
    // 这里先用长度为 1 的数组占位
    std::atomic<Node*> next_[1];
};

}  // namespace massdb

#endif  // MASSDB_SKIPTLIST_H
