/*
 * Created by Costa Bushnaq
 *
 * 19-01-2023 @ 01:40:26
 *
 * see LICENSE
*/

#ifndef DRIVER_HPP
#define DRIVER_HPP

/* TODO kqueue implementation */

#ifdef __linux__
    #include <sys/epoll.h>
#endif

#include "ykz.hpp"

namespace ykz {

#ifdef __linux__
    typedef struct epoll_event Event;
#endif

enum {
    EVF_READ   = (1 << 0),
    EVF_WRITE  = (1 << 1),
    EVF_SHARED = (1 << 2)
};

s32  driver_new();
void *driver_event_get_data(const Event&);
bool driver_event_is_error(const Event&);
bool driver_add_event(s32, s32, u16, void const*);
bool driver_mod_event(s32, s32, u16, void const*);
bool driver_poll_events(s32, Event*, u64);
void driver_thread(s32, std::atomic<bool>&, Protocol&);

} // namespace ykz

#endif /* DRIVER_HPP */
