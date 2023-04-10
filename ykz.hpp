/*
 * Created by Costa Bushnaq
 *
 * 15-01-2023 @ 21:35:40
 *
 * see LICENSE
*/

#ifndef YKZ_HPP
#define YKZ_HPP

#include <atomic>
#include <cstdint>
#include <thread>

namespace ykz {

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "ykz.config.hpp"

enum proto_result {
    PRS_SWITCH_MODE = 0,
    PRS_SUGGEST_WAITING,
    PRS_INTERNAL_ERROR,
};

struct Protocol {
    void (*init)();
    proto_result (*okay)(void *);
    void (*goodbye)(void *);
    void (*cleanup)();
};

enum connection_state {
    CON_VACANT = 0,
    CON_RECV_HEADER,
    CON_SEND_HEADER,
    CON_SEND_FILE,
};

struct Guest {
    s32 socket;
    connection_state s;
    char header[HEADER_LENGTH];
    u32 rcur;
    u32 wcur;
    struct File {
        bool is_pending;
        s32 fd;
        s64 lower;
        s64 upper;
        s64 size;
        s64 last_modified;
    } file;
};

struct Host {
    u32 address;
    u16 port;
    s32 socket;
    std::atomic<bool> is_running;
    std::thread workers[DRIVERS_PER_HOST];
};

void host_start(Host&, Protocol&, u32, u16);
void host_stop(Host&);

} // namespace ykz

#endif /* YKZ_HPP */
