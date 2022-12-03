/*
 * Created by Costa Bushnaq
 *
 * 04-10-2022 @ 17:19:41
*/

#include "Host.hpp"
#include "utils.hpp"

#include "og/internal.hpp"
#include "og/TcpStream.hpp"

#include <iostream>
#include <fcntl.h>

namespace ykz {

static void
server_event(s32 handle, og::Poll &poll, og::Event &event, Guest_Info *slots)
{
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
    

    u32 id = 0;
    for (; id < NB_GUESTS; id++) {
        if (slots[id].socketfd == og::k_bad_socketfd) {
            break;
        }
    }

    if (id == NB_GUESTS) {
        YKZ_LOG("NO MORE ROOM\n");
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
    YKZ_LOG("Accepted new client\n");
}

static void
guest_event(
    Protocol *proto, og::Poll &poll, og::Event event,
    Guest_Info &guest, Guest_Info::state &s
) {
    auto got_syserror{false};
    auto id = event.id();

    if (event.is_error()) {
        got_syserror = true;
    } else if (s == Guest_Info::RX_HEADER) {
        switch (data::rx_header(guest)) {
        case data::e_nothing:
            return;
        case data::e_made_progress:
        case data::e_all_done:
            if (!proto->okay(guest)) {
                if (poll.refresh(guest.socketfd, id, og::Poll::e_write) < 0)
                    got_syserror = true;
            s = Guest_Info::TX_HEADER; // we mark as ready for response
            } return;
        case data::e_connection_closed:
            break;
        case data::e_error:
            got_syserror = true;
        }
    } else if (s == Guest_Info::TX_HEADER) {
        switch (data::tx_response(guest)) {
        case data::e_nothing:
        case data::e_made_progress:
            return;
        case data::e_all_done:
            if (poll.refresh(guest.socketfd, id, og::Poll::e_read) < 0)
                got_syserror = true;
            YKZ_CX_REFRESH(guest);
            return;
        case data::e_connection_closed:
            break;
        case data::e_error:
            got_syserror = true;
        }
    }

    if (got_syserror)
        YKZ_LOG("System error: %s\n", strerror(errno));
    og::intl::close(guest.socketfd);
    YKZ_CX_RESET(guest);
    YKZ_LOG("Dropped a client\n");
}

static void host_worker_fn(s32 handle, Protocol *proto)
{
    og::Poll poll;
    og::Events events;
    auto slots  = new Guest_Info[NB_GUESTS];
    auto states = new Guest_Info::state[NB_GUESTS];

    if (poll.add(handle, NB_GUESTS, og::Poll::e_read|og::Poll::e_shared) < 0) {
        YKZ_LOG("POLL ADD ERROR: %s\n", strerror(errno));
        // @Todo log
        return;
    }

    for (;;) {
        YKZ_LOG("WAIT FOR EVENT\n");
        if (poll.poll(events, -1) < 0) {
            continue;
        }

        for (auto event : events) {
            switch (event.id()) {
            case NB_GUESTS:
                server_event(handle, poll, event, slots);
                break;
            default:
                guest_event(proto, poll, event, 
                            slots[event.id()], states[event.id()]);
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
