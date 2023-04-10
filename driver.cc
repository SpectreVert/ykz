/*
 * Created by Costa Bushnaq
 *
 * 19-01-2023 @ 01:37:31
*/

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include "driver.hpp"
#include "utils.hpp"

namespace ykz {

s32 driver_new()
{
    s32 driver;

    #ifdef __linux__
        if ((driver = epoll_create1(0)) < 0) {
            log("epoll_create1:");
            return -1;
        }
    #endif

    return driver;
}

void *driver_event_get_data(Event const &event)
{
    #ifdef __linux__
        return event.data.ptr;
    #endif
}

bool driver_event_is_error(Event const &event)
{
    #ifdef __linux__
        return event.events & EPOLLERR;
    #endif
}

bool driver_add_event(s32 driver, s32 socket, u16 flags, void const *data)
{
    u32 bits;

    #ifdef __linux__
        epoll_event event{ 0 };

        if (flags & EVF_SHARED) {
            bits = EPOLLEXCLUSIVE;
        } else {
            bits = EPOLLET;
        }

        if (flags & EVF_READ) {
            bits |= EPOLLIN;
        }

        if (flags & EVF_WRITE) {
            bits |= EPOLLOUT;
        }

        event.events = bits;
        event.data.ptr = (void*)data;

        if (epoll_ctl(driver, EPOLL_CTL_ADD, socket, &event) < 0) {
            log("epoll_ctl:");
            return false;
        }
    #endif

    return true;
}

bool driver_mod_event(s32 driver, s32 socket, u16 flags, void const *data)
{
    u32 bits{ EPOLLET };

    #ifdef __linux__
        epoll_event event{ 0 };

        if (flags & EVF_READ) {
            bits |= EPOLLIN;
        }
        if (flags & EVF_WRITE) {
            bits |= EPOLLOUT;
        }

        event.events = bits;
        event.data.ptr = (void*)data;

        if (epoll_ctl(driver, EPOLL_CTL_MOD, socket, &event) < 0) {
            log("epoll_ctl:");
            return false;
        }
    #endif

    return true;
}

bool driver_poll_events(s32 driver, Event *events, u64 nb_events)
{
    std::memset((void*)events, 0, sizeof(Event) * nb_events);

    #ifdef __linux__
    for (;;) {
        s32 nb = epoll_wait(driver, events, nb_events, 2 * 1000);

        /* timeout expired or we got something; go back to driver_thread() */
        if (nb >= 0) { break; }

        /* if we got an error, hope it's just an interupt */
        if (nb == -1) {
            if (errno == EINTR) { continue; }
            else {
                log("epoll_wait:");
                return false;
            }
        }
    }
    #endif

    return true;
}

static bool driver_connection_accept(
    s32 driver, s32 socket,
    Guest (&guests)[GUESTS_PER_DRIVER])
{
    s32 new_socket;
    u64 new_idx;
    char addr_str[INET_ADDRSTRLEN];
    struct sockaddr_in s{ 0 };
    socklen_t len{ sizeof(struct sockaddr_in) };

    if ((new_socket = accept(socket, (sockaddr*)&s, &len)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;
        } else {
            log("accept:");
            return false;
        }
    }

    if (set_nonblock(new_socket, 1)) {
        log("ioctl:");
        close(new_socket);
        return true;
    }

    /* get the listening address to string form */
    ipv4_to_string(s.sin_addr.s_addr, addr_str);

    /* look for a free slot in the guest array */
    for (new_idx = 0; new_idx < GUESTS_PER_DRIVER; new_idx++) {
        if (guests[new_idx].s == CON_VACANT) {
            break;
        }
    }

    /* @Todo: improve policy for no free slots */
    /* as of right now if no slots are free, we close the connection */
    if (new_idx == GUESTS_PER_DRIVER) {
        log("[%s:%hu] REFUSE", 0, addr_str, ntohs(s.sin_port));
        close(new_socket);
        return true;
    }

    if (!driver_add_event(driver, new_socket, EVF_READ, &guests[new_idx])) {
        close(new_socket);
        return true;
    }

    guests[new_idx].socket = new_socket;
    guests[new_idx].s = CON_RECV_HEADER;

    log("[#%d] [%s:%hu] ACCEPT", new_socket, addr_str, ntohs(s.sin_port));

    return true;
}

static void driver_guest_serve(s32 driver, Event &event, Protocol &p)
{
    Guest *guest = (Guest*)driver_event_get_data(event);

    if (driver_event_is_error(event)) {
        goto drop;
    }

    /* Driver operations */
    switch (guest->s) {
    case CON_RECV_HEADER:
        /* Socket (input) operation */
        switch (recv_header(*guest)) {
        case IOR_AGAIN:
        case IOR_BUFFER_LIMIT:
            break;
        case IOR_CONNECTION_CLOSED:
            goto drop;
        case IOR_ERROR:
            log("recv:");
            goto drop;
        }
        /* Protocol operation */
        switch (p.okay(guest)) {
        case PRS_SWITCH_MODE:
            if (!driver_mod_event(driver, guest->socket, EVF_WRITE, guest)) {
                goto drop;
            }
            guest->s = CON_SEND_HEADER;
            break;
        case PRS_SUGGEST_WAITING:
            return;
        case PRS_INTERNAL_ERROR:
            goto drop;
        }
        /* FALLTHRU */
    case CON_SEND_HEADER:
        /* Socket (output) operation */
        switch (send_header(*guest)) {
        case IOR_AGAIN:
            return;
        case IOR_BUFFER_LIMIT:
            if (!guest->file.is_pending) { goto cleanup; }
            guest->s = CON_SEND_FILE;
            break;
        case IOR_CONNECTION_CLOSED:
            goto drop;
        case IOR_ERROR:
            log("send:");
            goto drop;
        }
        /* FALLTHRU */
    case CON_SEND_FILE:
        /* @Todo: apply check of CLOSE_ON_RESPONSE */
        switch (send_file(*guest)) {
        case IOR_AGAIN:
            return;
        case IOR_BUFFER_LIMIT:
            goto cleanup;
        case IOR_CONNECTION_CLOSED:
            goto drop;
        case IOR_ERROR:
            log("sendfile:");
            goto drop;
        }
    default:
        if (!guest->s) { log("This should not happen\n"); }
    };

cleanup:
    if (!driver_mod_event(driver, guest->socket, EVF_READ, guest)) { goto drop; }
    if (guest->file.fd > 0) { close(guest->file.fd); }
    memset(guest->header, 0, HEADER_LENGTH);
    memset(&guest->file, 0, sizeof(Guest::File));
    guest->rcur = guest->wcur = 0;
    guest->s = CON_RECV_HEADER;
    return;

drop:
    p.goodbye(guest);
    close(guest->socket);
    if (guest->file.fd > 0) { close(guest->file.fd); }
    log("[#%d] CLOSE", guest->socket);
    memset(guest, 0, sizeof(Guest));
}

void driver_thread(s32 socket, std::atomic<bool> &all_good, Protocol &p)
{
    s32 driver;
    sigset_t signal_set;
    Event events[EVENTS_PER_BATCH]{ 0 };
    Guest guests[GUESTS_PER_DRIVER]{ 0 };

    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGHUP);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGQUIT);

    if (pthread_sigmask(SIG_BLOCK, &signal_set, 0x0) < 0) {
        log("pthread_sigmask:");
        return;
    }

    if ((driver = driver_new()) < 0) {
        return;
    }

    if (!driver_add_event(driver, socket, EVF_READ|EVF_SHARED, 0x0)) {
        return;
    }

    p.init();

    do {
        if (!driver_poll_events(driver, events, EVENTS_PER_BATCH)) {
            return;
        }

        for (u32 i = 0; i < EVENTS_PER_BATCH && events[i].events; i++) {
            if (!driver_event_get_data(events[i])) {
                driver_connection_accept(driver, socket, guests);
            } else {
                driver_guest_serve(driver, events[i], p);
            }
        }

    } while (all_good);

    p.cleanup();
}

} // namespace ykz
