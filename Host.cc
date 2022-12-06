/*
 * Created by Costa Bushnaq
 *
 * 04-10-2022 @ 17:19:41
*/

#include "Host.hpp"
#include "utils.hpp"

#include "og/internal.hpp"
#include "og/TcpStream.hpp"

#include <algorithm>
#include <iostream>
#include <fcntl.h>

namespace ykz {

static void
server_event(
    s32 handle, og::Poll &poll, og::Event &event,
    Guest_Info *slots, Guest_Info::state *states
){
    s32 newsock;
    og::SocketAddr addr{{0, 0, 0, 0}, 0};

    if (!event.is_readable()) {
        // Log error
        return;
    }

    if ((newsock = og::intl::accept(handle, addr)) == og::k_bad_socketfd) {
        YKZ_LOG("Accept error: %s\n", strerror(errno));
        // @Todo @Log accept error
        return;
    }
    

    u64 id = 0;
    for (; id < NB_GUESTS; id++) {
        if (states[id] == Guest_Info::VACANT) {
            break;
        }
    }

    if (id == NB_GUESTS) {
        YKZ_LOG("NO MORE ROOM IN HELL\n");
        og::intl::close(newsock);
        return;
    }

    if (poll.add(newsock, id, og::Poll::e_read) < 0) {
        YKZ_LOG("Poll add: %s\n", strerror(errno));
        og::intl::close(newsock);
        return;
    }

    YKZ_CX_RESET(slots[id]);
    slots[id].socketfd = newsock;
    states[id] = Guest_Info::RX_MODE;
    YKZ_LOG("Accepted new client\n");
}

static void
guest_event(
    Protocol *proto, og::Poll &poll, og::Event event,
    Guest_Info &guest, Guest_Info::state &s
){
    bool header_filled{false};
    auto id = event.id();

    if (event.is_error())
        goto error;

    if (event.is_readable()) {
        switch (data::rx_header(guest)) {
        case data::e_nothing:
            break;
        case data::e_all_done:
            header_filled = true;
        case data::e_made_progress:
            if (!proto->okay(guest)) {
                if (poll.refresh(guest.socketfd, id, og::Poll::e_write)) {
                    goto error;
                }
                s = Guest_Info::TX_MODE;
                break;
            } else if (header_filled) {
                goto drop;
            } break;
        case data::e_connection_closed:
            goto drop;
        case data::e_error:
            goto error;
        }
    }

    if (event.is_writable()) {
        switch (data::tx_response(guest)) {
        case data::e_nothing:
        case data::e_made_progress:
            break;
        case data::e_all_done:
            if (!poll.refresh(guest.socketfd, id, og::Poll::e_read)) {
                YKZ_CX_REFRESH(guest);
                s = Guest_Info::RX_MODE;
                break;
            }
            goto error;
        case data::e_connection_closed:
            goto drop;
        case data::e_error:
            goto error;
        }
    }
    return;

error:
    std::cout << "System error: " << strerror(errno) << "\n";
drop:
    YKZ_LOG("Drop client\n");
    og::intl::close(guest.socketfd);
    YKZ_CX_RESET(guest);
    s = Guest_Info::VACANT;
}

static void host_worker_fn(s32 handle, Protocol *proto)
{
    og::Poll poll;
    og::Events events;
    Guest_Info slots[NB_GUESTS];
    Guest_Info::state states[NB_GUESTS];

    if (poll.add(handle, NB_GUESTS, og::Poll::e_read|og::Poll::e_shared) < 0) {
        YKZ_LOG("POLL ADD ERROR: %s\n", strerror(errno));
        // @Todo log
        return;
    }

    for (u64 i = 0; i < NB_GUESTS; i++) {
        states[i] = Guest_Info::VACANT;
    }

    for (;;) {
        YKZ_LOG("WAIT FOR EVENT\n");
        if (poll.poll(events, -1) < 0) {
            continue;
        }

        for (auto event : events) {
            switch (event.id()) {
            case NB_GUESTS:
                server_event(handle, poll, event, slots, states);
                break;
            default:
                guest_event(proto, poll, event, slots[event.id()], states[event.id()]);
                break;
            }
        }
    }
}

void Host::start(og::SocketAddr &addr, Protocol *proto)
{
    // @Refactor I think we would like to run this hook
    // once for every worker thread.
    proto->init();

    if (m_insock.bind(addr) < 0) {
        YKZ_LOG("Bind error: %s\n", strerror(errno));
        return;
    }

    if (m_insock.listen(SOMAXCONN) < 0) {
        YKZ_LOG("Listen error: %s\n", strerror(errno));
        return;
    }

    YKZ_LOG("Preparing to serve %s:%d\n",
            addr.addr.v4.ip_host_order().to_string().c_str(),
            addr.addr.v4.port_host_order()
    );

    for (u32 i = 0; i < NB_WORKERS; i++)
        m_workers[i] = std::thread(host_worker_fn, m_insock.handle(), proto);

    this->is_started = true;
    // @Todo log server started ; join or continue.
}

void Host::join()
{
    if (!is_started)
        return;

    for (u32 i = 0; i < NB_WORKERS; i++)
    {
        if (m_workers[i].joinable())
            m_workers[i].join();
    }
}

} // namespace ykz
