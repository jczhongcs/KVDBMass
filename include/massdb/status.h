//
// Created by Xsakura on 2023/3/30.
//

#ifndef MASSDB_INCLUDE_STATUS_H_
#define MASSDB_INCLUDE_STATUS_H_

#include <string>

#include "massdb/slice.h"

namespace massdb {

class Status {
public:
    // 创建一个 Ok 的 status。
    Status() noexcept : state_(nullptr) {}
    ~Status() { delete[] state_; }

    Status(const Status& rhs) {
        state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
    }
    Status& operator=(const Status& rhs) {
        // The following condition catches both aliasing (when this == &rhs),
        // and the common case where both rhs and *this are ok.
        if (state_ != rhs.state_) {
            delete[] state_;
            state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
        }
        return *this;
    }

    Status(Status&& rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
    Status& operator=(Status&& rhs) noexcept {
        std::swap(state_, rhs.state_);
        return *this;
    }

    // 返回 Ok 状态
    static Status Ok() { return Status(); }

    // 返回相应错误类型及信息
    static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kNotFound, msg, msg2);
    }
    static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kCorruption, msg, msg2);
    }
    static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kNotSupported, msg, msg2);
    }
    static Status InvalidArgument(const Slice& msg,
                                  const Slice& msg2 = Slice()) {
        return Status(kInvalidArgument, msg, msg2);
    }
    static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(kIOError, msg, msg2);
    }

    // 判断当前状态
    bool IsOk() const { return (state_ == nullptr); }
    bool IsNotFound() const { return code() == kNotFound; }
    bool IsCorruption() const { return code() == kCorruption; }
    bool IsIOError() const { return code() == kIOError; }
    bool IsNotSupportedError() const { return code() == kNotSupported; }
    bool IsInvalidArgument() const { return code() == kInvalidArgument; }

    // 将错误信息转换为合适的 string 类型
    std::string ToString() const;

private:
    // 状态码
    enum Code {
        kOk = 0,               // 成功
        kNotFound = 1,         // 文件未找到
        kCorruption = 2,       // 中断错误
        kNotSupported = 3,     // 不支持
        kInvalidArgument = 4,  // 参数不合法
        kIOError = 5           // IO 错误
    };

    // 获取 state 的第 5 个字节，即状态码
    Code code() const {
        return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
    }

    // 将状态码和提示信息放入 state_ 中
    Status(Code code, const Slice& msg, const Slice& msg2);

    // 深拷贝 state 中的内容
    static const char* CopyState(const char* state);

    // Ok 状态时 state_ 为 nullptr。其他情况下，state 遵循一下格式
    //    state_[0..3] == length of message
    //    state_[4]    == code
    //    state_[5..]  == message
    const char* state_;
};

}  // namespace massdb

#endif  // MASSDB_INCLUDE_STATUS_H_
