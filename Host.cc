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
server_event(s32 handle, Guest *guests, og::Poll &poll, og::Event &event)
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
    for (; id < GUESTS_MAX; id++) {
        if (guests[id].socketfd == og::k_bad_socketfd) {
            break;
        }
    }

    if (id == GUESTS_MAX) {
        YKZ_LOG("NO MORE ROOM\n");
        og::intl::close(newsock);
        return;
    }

    if (poll.add(newsock, id, og::Poll::e_read) < 0) {
        YKZ_LOG("Poll add: %s\n", strerror(errno));
        og::intl::close(newsock);
        return;
    }

    YKZ_CX_RESET(guests[id]);
    guests[id].socketfd = newsock;
    YKZ_LOG("Accepted new client\n");
}

static void
guest_event(Protocol *proto, Guest &guest, og::Poll &poll, og::Event event)
{

    auto id = event.id();

    if (event.is_error())
        goto error;

    if (event.is_readable()) {
        switch (data::rx_buffer(guest)) {
        case data::e_nothing:
            break;
        case data::e_made_progress:
        case data::e_all_done:
            if (proto->okay(guest)) {
                if (poll.refresh(guest.socketfd, id, og::Poll::e_write) < 0)
                    goto error;
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
            if (poll.refresh(guest.socketfd, id, og::Poll::e_read) < 0)
                goto error;
            YKZ_CX_REFRESH(guest);
            break;
        case data::e_connection_closed:
            goto drop;
        case data::e_error:
            goto error;
        }
    }
    return;
error:
    std::cout << "got error\n";
drop:
    og::intl::close(guest.socketfd);
    YKZ_CX_RESET(guest);
}

static void host_worker_fn(s32 handle, Protocol *proto)
{
    og::Poll poll;
    og::Events events;
    Guest *guests = new Guest[GUESTS_MAX];

    if (poll.add(handle, GUESTS_MAX, og::Poll::e_read|og::Poll::e_shared) < 0) {
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
            case GUESTS_MAX:
                server_event(handle, guests, poll, event);
                break;
            default:
                guest_event(proto, guests[event.id()], poll, event);
                break;
            }
        }
    }
}

void Host::start(og::SocketAddr &addr, Protocol *proto)
{
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
