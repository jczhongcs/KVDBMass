//
// Created by Xsakura on 2023/3/30.
//

#ifndef MASSDB_INCLUDE_SLICE_H
#define MASSDB_INCLUDE_SLICE_H

#include <cassert>
#include <cstring>
#include <string>

namespace massdb {
// 对字符串进行了封装，以实现更方便的字符串操作
class Slice {
public:
    // 创建一个空的 slice
    Slice() : data_(""), size_(0) {}
    ~Slice() {
        delete[] data_;
        size_ = 0;
    }

    Slice(const char* str, size_t n) : data_(str), size_(n) {}

    Slice(const std::string& str) : data_(str.c_str()), size_(str.size()) {}

    Slice(const char* str) : data_(str), size_(strlen(str)) {}

    // 允许同类型之间的复制
    Slice(const Slice&) = default;
    Slice& operator=(const Slice&) = default;

    // 返回指向 data 开头的指针
    const char* data() const { return data_; }

    // 返回 data 的长度
    size_t size() const { return size_; }

    // 如果 data 的长度为 0 的话返回 true
    bool empty() const { return size_ == 0; }

    // 返回 data 的第 i 个字节
    // REQUIRES: n < size()
    char operator[](size_t n) const {
        assert(n < size());
        return data_[n];
    }

    // 清空 slice
    void clear() {
        data_ = "";
        size_ = 0;
    }

    // 丢弃 slice 的前 n 个字节
    void remove_prefix(size_t n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }

    // 将 slice 转换为 string 类型
    std::string to_string() const { return std::string(data_, size_); }

    // 三次比较。 Returns value:
    //   <  0 iff "*this" <  "b",
    //   == 0 iff "*this" == "b",
    //   >  0 iff "*this" >  "b"
    // iff 表示当且仅当
    int compare(const Slice& b) const {
        const size_t min_len = (size_ < b.size()) ? size_ : b.size();
        int res = memcmp(data_, b.data(), min_len);
        if (res == 0) {
            if (size_ < b.size()) {
                return -1;
            } else if (size_ > b.size()) {
                return +1;
            }
        }
        return res;
    }

    // 当且仅当 x 是 data 的前缀时返回 true
    bool starts_with(const Slice& x) const {
        return ((size_ >= x.size())) && (memcmp(data_, x.data_, x.size_) == 0);
    }

private:
    const char* data_;
    size_t size_;
};

inline bool operator==(const Slice& x, const Slice& y) {
    return ((x.size() == y.size()) &&
            (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) { return !(x == y); }

}  // namespace massdb

#endif  // MASSDB_INCLUDE_SLICE_H
