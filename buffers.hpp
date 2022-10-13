/*
 * Created by Costa Bushnaq
 *
 * 28-09-2022 @ 22:27:29
 *
 * see LICENSE
*/

#ifndef YKZ_BUFFERS_HPP_
#define YKZ_BUFFERS_HPP_

#include "./ykz.config.hpp"

#include "og/Buffer.hpp"

#include "sys/socket.h"

#include <cassert>
#include <cstring>

namespace ykz {

using og::u64;
using og::s64;
using og::s32;

// Frame is a higher level object used to do all sorts of operations
// on received or to-be-sent messages. Partial send is also one of the
// problems that aim to be fixed with Frame.
// TODO: Faire un Buffer avec une interface commune: data(), capacity(), cursor(), free_bytes()... etc. Frame serait une implémentation fixed cursor, il faudrait aussi un StreamBuf ou jsp quoi. Avec un data de taille arbitraire par exemple.
//
// acheter poster
//
// Do we want to override previously writen bytes??
// and how?
//

struct IMemoryBuffer {
    virtual ~IMemoryBuffer() = default;

    //! reset the internal cursor, and buffer memory
    virtual void reset() = 0;

    //! return the number of bytes before the cursor
    virtual u64 size() const = 0;
    //! return the number of bytes with and after the cursor
    virtual u64 vacant() const = 0;

    //! return a const ptr to the start of the data segment
    virtual char const *view() const = 0;
    //! return a ptr to the cursor offset in the data segment
    virtual char *data() = 0;
};

template<u64 N>
struct StaticBuffer : public IMemoryBuffer {
    char buf[N];
    u64 const capacity{N};
    u64 cursor{0};

    void reset() {
        cursor = 0;
        std::memset(buf, 0, capacity);
    }

    u64 size() const {
        assert(cursor <= capacity);
        return cursor;
    }
    u64 vacant() const {
        assert(cursor <= capacity);
        return capacity - cursor;
    }

    char const *view() const { return const_cast<char const*>(buf); }
    char *data() {
        assert(cursor <= capacity);
        return buf + cursor;
    }
};

struct DynamicBuffer : public IMemoryBuffer {
    char *buf{nullptr};
    u64 const capacity;
    u64 cursor{0};

    ~DynamicBuffer() { delete[] buf; }
    DynamicBuffer(u64 t_capacity) : capacity(t_capacity) {
        buf = new char[capacity];
    }

    void reset() {
        cursor = 0;
        std::memset(buf, 0, capacity);
    }

    u64 size() const {
        assert(cursor <= capacity);
        return cursor;
    }
    u64 vacant() const {
        assert(cursor <= capacity);
        return capacity - cursor;
    }

    char const *view() const { return const_cast<char const*>(buf); }
    char *data() {
        assert(cursor <= capacity);
        return buf + cursor;
    }
};

} // namespace ykz

#endif /* YKZ_BUFFERS_HPP_ */
