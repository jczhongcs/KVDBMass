//
// Created by Xsakura on 2023/3/31.
//

#ifndef MASSDB_INCLUDE_OPTIONS_H
#define MASSDB_INCLUDE_OPTIONS_H

#include <cstddef>

namespace massdb {

// DB 内容存储在一组块中，每个块都包含一系列键值对。
// 每个块在存储到文件之前可能会被压缩。
// 以下枚举类描述用于压缩块的压缩方法
enum CompressionType {
    // 注意：不要更改现有条目的值，因为这些值是磁盘上持久格式的一部分。
    kNoCompression = 0x0,     // 不压缩
    kSnappyCompression = 0x1  // 使用 Snappy 压缩算法
};

// 用于控制数据库行为的选项（传递给 DB:Open()）
struct Options {
    // 创建一个 Options 对象，其中所有字段都具有默认值。
    Options();

    // -------------------
    // 影响数据库行为的参数

    // If true，缺失数据库的话将会创建一个新的数据库
    bool create_if_missing = false;

    // If true，如果数据库存在的话将会报错
    bool error_if_exists = false;

    // If true，则其处理的数据进行积极检查并在检测到任何错误时提前停止。
    // 这可能会产生意想不到的影响：
    // 例如，一个单个的数据库条目损坏可能导致大量的条目变得无法读取，
    // 或者整个数据库无法打开。
    //
    // paranoid 偏执狂
    bool paranoid_checks = false;

    // -------------------
    // 影响数据库性能的参数

    // 内存中的写缓存大小。
    // 在将未排序的日志写入磁盘之前，需要在内存中累积多少数据量，
    // 才会将其转换为已排序的磁盘文件。
    //
    // 较大的 write_buffer_size 值可以提高性能，特别是在大量数据导入时。
    // 同时，由于在内存中最多可以同时存在两个写缓存，因此可能需要调整此参数以控制内存使用。
    // 此外，较大的内存写缓存数据量会导致数据库下次打开时的恢复时间更长。
    //
    // 这里的两个写缓存是指正在写入的缓存和下一个待写入的缓存
    // 当写入操作开始时，数据首先会被写入当前正在写入的缓存，
    // 当该缓存的大小达到 write_buffer_size 设定值时，
    // DB 会将该缓存中的数据写入磁盘文件并清空缓存；
    // 同时，下一个待写入的缓存已经被分配好并准备就绪，
    // 此时写入操作可以继续在未满的缓存中进行。
    // 在未来的写入操作中，已经写入到磁盘文件中的数据会按顺序合并成更大的有序文件。
    // 这样做的目的是减少磁盘写入操作的次数，提高写入操作的效率。
    size_t write_buffer_size = 4 * 1024 * 1024;

    // DB 能打开文件的数量
    // 在运行期间可能会打开许多文件，例如数据文件、日志文件、元数据文件等等。
    // max_open_files 就是用来限制数据库可以同时打开的文件数目，
    // 以控制内存使用和提高性能。
    // 如果数据集非常大，可能需要适当增大 max_open_files参数的值，
    // 以避免过多文件被频繁地打开和关闭，从而提高读写操作的性能。
    int max_open_files = 1000;

    // 控制 blocks（用户的数据存储在一组 blocks 中，一个 block
    // 是最小的读取单元） 如果非空，使用指定的 block 缓存
    // 如果为空，数据库会默认创建并使用一个 8MB 的内部缓存
    //
    //    Cache* block_cache = nullptr;

    // 每个块中打包的用户数据的大概大小。
    // 请注意，此处指定的块大小对应于未压缩的数据。
    // 如果启用了压缩，则从磁盘读取的实际单元大小可能会更小。
    // 此参数可以动态更改。
    size_t block_size = 4 * 1024;

    // 对于压缩 keys 的重启点
    // 具体可以阅读 leveldb 相关键（key）压缩知识
    // 此参数可以动态更改。大多数客户端应该保持此参数不变。
    int block_restart_interval = 16;

    // Leveldb 在切换到新文件之前会将文件写入最多这么多字节。
    // 大多数客户端应该保持此参数不变。
    // 但是，如果您的文件系统使用较大文件更有效，则可以考虑增加该值。
    // 缺点将是更长时间的压实和更长的延迟/性能中断。
    // 另一个增加此参数的原因可能是当您首次填充大型数据库时。
    size_t max_file_size = 2 * 1024 * 1024;

    // 使用指定的压缩算法压缩块。此参数可以动态更改。
    //
    // 默认值：kSnappyCompression，提供轻量级但快速的压缩。
    //
    // 在 Intel(R) Core(TM)2 2.4GHz上，kSnappyCompression 的典型速度为：
    //      ~200-500MB/s 压缩
    //      ~400-800MB/s 解压缩
    //
    // 注意：这些速度显着快于大多数持久存储速度，通常不值得切换到 kNoCompression
    // 即使输入数据不可压缩，kSnappyCompression 也将高效检测到这一点，
    // 并切换到未压缩模式。
    //     CompressionType compression = kSnappyCompression;

    // 如果非空，则使用指定的过滤器策略以减少磁盘读取。
    //    const FilterPolicy* filter_policy = nullptr;
};

// 控制读操作的选项
struct ReadOptions {
    // If true, 从底层存储读取的所有数据都将与相应的校验和进行验证。
    bool verify_checksums = false;

    // 在此迭代中读取的数据是否应缓存在内存中？
    // 调用者可能希望将此字段设置为 false 以进行批量扫描。
    bool fill_cache = true;
};

// 控制写操作的选项
struct WriteOptions {
    WriteOptions() = default;

    // If true，在写操作完成之前，写入将从操作系统的缓冲区缓存中被刷新
    // （通过调用WritableFile::Sync()函数）。
    // 如果此标志设置为true，则写操作将变慢。
    //
    // If false，并且机器崩溃，一些最近的写操作可能会丢失。
    // 请注意，如果只是进程崩溃（即机器不重启），
    // 即使sync=false，也不会丢失写操作。
    //
    // 换句话说，sync==false 的 DB 写操作与 "write()"
    // 系统调用具有类似的崩溃语义。 sync=true 的 DB 写操作与 "write()"
    // 系统调用后跟 "fsync()" 具有类似的崩溃语义。
    bool sync = false;
};

}  // namespace massdb

#endif  // MASSDB_INCLUDE_OPTIONS_H
