//
// Created by Xsakura on 2023/4/1.
//

#include "massdb/comparator.h"

#include "massdb/slice.h"

#include "util/no_destructor.h"

namespace massdb {

namespace {

class BytewiseComparatorImpl : public Comparator {
public:
    BytewiseComparatorImpl() = default;

    const char* Name() const override { return "massdb.BytewiseComparator"; }

    int Compare(const Slice& a, const Slice& b) const override {
        return a.compare(b);
    }

    // 获取 start 和 limit 的共同前缀长度
    // 截断 start 共同前缀长度后面的字符
    void FindShortestSeparator(std::string* start,
                               const Slice& limit) const override {
        // 获取共同前缀的长度
        size_t min_length = std::min(start->size(), limit.size());
        size_t diff_index = 0;
        while ((diff_index < min_length) &&
               ((*start)[diff_index] == limit[diff_index])) {
            diff_index++;
        }

        if (diff_index >= min_length) {
        } else {
            uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
            if (diff_byte < static_cast<uint8_t>(0xff) &&
                diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
                (*start)[diff_index]++;
                start->resize(diff_index + 1);
                assert(Compare(*start, limit) < 0);
            }
        }
    }

    void FindShortestSuccessor(std::string* key) const override {
        // 找到第一个能自增的字符
        size_t n = key->size();
        for (size_t i = 0; i < n; i++) {
            const uint8_t byte = (*key)[i];
            if (byte != static_cast<uint8_t>(0xff)) {
                (*key)[i] = byte + 1;
                key->resize(i + 1);
                return;
            }
        }
    }
};

}  // namespace

const Comparator* BytewiseComparator() {
    static NoDestructor<BytewiseComparatorImpl> singleton;
    return singleton.get();
}
}  // namespace massdb
