//
// Created by Xsakura on 2023/4/1.
//

#ifndef MASSDB_UNTIL_NO_DESTRUCOTR_H
#define MASSDB_UNTIL_NO_DESTRUCOTR_H

#include <type_traits>
#include <utility>

namespace massdb {

// Wraps an instance whose destructor is never called.
// 封装永远不用调用析构器的类
//
// 用于函数级别的静态变量
template <typename InstanceType>
class NoDestructor {
public:
    // template <typename... ConstructorArgTypes>
    // 是一个可变参数模板参数列表，它表示 ConstructorArgTypes
    // 是一个类型参数包，可以同时指定多个类型参数。
    // 在模板实例化时，可以将任意数量的类型作为模板参数进行传递。
    template <typename... ConstructorArgTypes>
    explicit NoDestructor(ConstructorArgTypes&&... construcotor_args) {
        // static_assert 第二个参数是当第一个参数为 false 的时候输出
        static_assert(
            sizeof(instance_storage_) >= sizeof(InstanceType),
            "instance_storage_ is not large enough to hold the instance");
        static_assert(
            alignof(decltype(instance_storage_)) >= alignof(InstanceType),
            "instance_storage_ does not meet the instance's alignment "
            "requirement");
        new (&instance_storage_) InstanceType(
            std::forward<ConstructorArgTypes>(construcotor_args)...);
    }

    ~NoDestructor() = default;

    NoDestructor(const NoDestructor&) = delete;
    NoDestructor& operator=(const NoDestructor&) = delete;

    InstanceType* get() {
        // reinterpret_cast 不进行类型检查的指针或者引用转换
        return reinterpret_cast<InstanceType*>(&instance_storage_);
    }

private:
    // aligned_storage 模板类，用于在内存中分配指定大小和对齐方式的存储空间。
    // template <std::size_t Len, std::size_t Align = /*default-alignment*/>
    // struct aligned_storage;
    //      Len:   所需存储空间的大小
    //      Align: 参数为表示所需存储空间的对齐方式
    // 由于 aligned_storage 类型是一个模板类型，
    // 而模板类型不能直接用于变量定义，因此需要使用 typename
    // 和 ::type 限定符来获取 aligned_storage::type 的实际类型
    //
    // alignof 是 C++11 引入的一种运算符，
    // 用于返回类型的对齐要求
    typename std::aligned_storage<
        sizeof(InstanceType), alignof(InstanceType)>::type instance_storage_;
};

}  // namespace massdb

#endif  // MASSDB_UNTIL_NO_DESTRUCOTR_H
