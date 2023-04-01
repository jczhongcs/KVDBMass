//
// Created by Xsakura on 2023/4/1.
//

#ifndef MASSDB_INCLUDE_COMPARATOR_H
#define MASSDB_INCLUDE_COMPARATOR_H

#include <string>

namespace massdb {

class Slice;

class Comparator {
public:
    virtual ~Comparator() = default;

    virtual int Compare(const Slice& a, const Slice& b) const = 0;

    virtual const char* Name() const = 0;

    virtual void FindShortestSeparator(std::string* start,
                                       const Slice& limit) const = 0;
    virtual void FindShortestSuccessor(std::string* key) const = 0;
};

const Comparator* BytewiseComparator();
}  // namespace massdb

#endif  // MASSDB_INCLUDE_COMPARATOR_H
