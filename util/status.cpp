//
// Created by Xsakura on 2023/3/30.
//

#include "massdb/status.h"

#include <cstdio>

namespace massdb {

std::string Status::ToString() const {
    if (state_ == nullptr) {
        return "OK";
    } else {
        char tmp[30];
        const char* type;
        switch (code()) {
            case kOk:
                type = "OK";
                break;
            case kNotFound:
                type = "NotFound: ";
                break;
            case kCorruption:
                type = "Corruption: ";
                break;
            case kNotSupported:
                type = "Not implemented: ";
                break;
            case kInvalidArgument:
                type = "Invalid argument: ";
                break;
            case kIOError:
                type = "IO error: ";
                break;
            default:
                // std::snprintf() 函数将指定格式的字符串放入到指定的缓冲区中
                std::snprintf(tmp, sizeof(tmp),
                              "Unknown code(%d): ", static_cast<int>(code()));
                type = tmp;
                break;
        }
        std::string result(type);
        uint32_t length;
        std::memcpy(&length, state_, sizeof(length));
        result.append(state_ + 5, length);
        return result;
    }
}

Status::Status(Status::Code code, const Slice& msg, const Slice& msg2) {
    assert(code != kOk);
    const uint32_t len1 = static_cast<uint32_t>(msg.size());
    const uint32_t len2 = static_cast<uint32_t>(msg2.size());
    // 这里多出来的 2 个字节可以看下面的 if 语句，用于插入一些间隔符号
    const uint32_t size = len1 + (len2 ? (2 + len2) : 0);
    // 向前预留 5 个字节
    char* result = new char[size + 5];
    std::memcpy(result, &size, sizeof(size));
    result[4] = static_cast<char>(code);
    std::memcpy(result + 5, msg.data(), len1);
    if (len2) {
        result[5 + len1] = ':';
        result[6 + len2] = ' ';
        std::memcpy(result + 7 + len1, msg2.data(), len2);
    }
    state_ = result;
}

const char* Status::CopyState(const char* state) {
    uint32_t size;
    // size 表示 message 的长度
    std::memcpy(&size, state, sizeof(size));
    // +5 表示预留存放 message 长度和状态码的空间
    char* result = new char[size + 5];
    std::memcpy(result, state, size + 5);
    return result;
}

}  // namespace massdb
