// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef MASSDB_UTIL_RANDOM_H_
#define MASSDB_UTIL_RANDOM_H_

#include <cstdint>

namespace massdb {

// A very simple random number generator.  Not especially good at
// generating truly random bits, but good enough for our needs in this
// package.
//
// 简单的随机数生成器，使用了线性同余生成器
// （Linear congruential generator，LCG）产生伪随机数。
// LCG 的生成过程可以表示为：
//      X[n+1] = (A * X[n] + C) mod M
// 其中，X[n] 表示第 n 个随机数，
//      A 为乘数，
//      C 为增量，
//      M 为模数，
//      X[0]称为种子(seed)。
// 这里没用到 C，或者说 C 为 0。
class Random {
private:
    uint32_t seed_;

public:
    explicit Random(uint32_t seed) : seed_(seed & 0x7fffffffu) {
        // Avoid bad seeds.
        // 因为在运算过程中用到了取余，所以生成的结果不能是 0 或者等于 M
        if (seed_ == 0 || seed_ == 2147483647L) {
            seed_ = 1;
        }
    }
    uint32_t Next() {
        static const uint32_t M = 2147483647L;  // 2^31-1
        static const uint64_t A = 16807;        // bits 14, 8, 7, 5, 2, 1, 0
        // We are computing
        //       seed_ = (seed_ * A) % M,    where M = 2^31-1
        //
        // seed_ must not be zero or M, or else all subsequent computed values
        // will be zero or M respectively.  For all other values, seed_ will end
        // up cycling through every number in [1,M-1]
        uint64_t product = seed_ * A;

        // Compute (product % M) using the fact that ((x << 31) % M) == x.
        // product 的高低位相加是增加随机信，让每一位都参与运算
        seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
        // The first reduction may overflow by 1 bit, so we may need to
        // repeat.  mod == M is not possible; using > allows the faster
        // sign-bit-based test.
        if (seed_ > M) {
            seed_ -= M;
        }
        return seed_;
    }
    // Returns a uniformly distributed value in the range [0..n-1]
    // REQUIRES: n > 0
    // 在 [0..n-1] 这个范围内随机取一个，随机分布。
    uint32_t Uniform(int n) { return Next() % n; }

    // Randomly returns true ~"1/n" of the time, and false otherwise.
    // REQUIRES: n > 0
    // 1/n 的概率返回 true，用于生成跳表节点的不同层
    bool OneIn(int n) { return (Next() % n) == 0; }

    // Skewed: pick "base" uniformly from range [0,max_log] and then
    // return "base" random bits.  The effect is to pick a number in the
    // range [0,2^max_log-1] with exponential bias towards smaller numbers.
    // Skewed 方法返回一个指数分布的随机整数，也就是说大概率返回较小的数
    // 两次离散分布实现指数分布，举例如下：
    //      1.首先 Uniform(100)，那么会生成 0 到 99 之间任意一个数，
    //      概率都是 1/100，即 0.01
    //      2.在上次 Uniform 的基础上，我再使用 Uniform，
    //      那么我获得 0 的概率可能是
    //          上次生成的是 0，这次是 0 的概率为 0.1；
    //          上次生成的是 1，这次是 0 的概率为 0.5；
    //          上次生成的是 100，这次是 0 的概率为 0.01
    //      总的概率是 （0.01 * 1）+ （0.01 * 0.5）+ ... + (0.01 * 0.01) = 0.05
    //      3.而要第二次生成 100，只有第一次生成 100 时，才可能生成。
    //      第二次生成 100 的总概率为 （0.01 * 0.01) = 0.0001
    uint32_t Skewed(int max_log) { return Uniform(1 << Uniform(max_log + 1)); }
};

}  // namespace massdb

#endif  // MASSDB_UTIL_RANDOM_H_
