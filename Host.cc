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

void Host::start(og::SocketAddr &addr)
{
    m_proto.init();
    m_listener = std::unique_ptr<og::TcpListener>(new og::TcpListener());

    // @TODO : remove this error checking verbosity with logger
    if (m_listener->bind(addr) < 0) {
        std::cout << "bind error\n";
        return;
    }

    if (m_listener->listen(SOMAXCONN) < 0) {
        std::cout << "listen error\n";
        return;
    }

    std::cout << "serving 127.0.0.1:6970 (or something else)\n";

    for (u32 i = 0; i < YKZ_MAX_CLIENTS; i++) {
        m_free_slots.push_back(i);
    }

    // @TODO Change these polling IDS
    if (m_poll.add(m_listener->handle(), 256, og::Poll::e_read|og::Poll::e_shared) < 0) {
        std::cout << "poll add error\n";
        return;
    }

    if (m_poll.add(m_pipe.in(), 257, og::Poll::e_read) < 0) {
        std::cout << "poll add blabla error\n";
        return;
    }

    m_thd = std::thread([this]() {
        og::Events events;

        for (;;) {
            std::cout << "Wait for event...\n";
            if (m_poll.poll(events, -1) < 0) {
                continue;
            }

            for (auto event : events) {
                switch (event.id()) {
                case 256:
                    on_server_event(event);
                    break;
                case 257:
                    // @TODO: write closing piece
                    std::cout << "Closing mofo server\n";
                    return;
                default:
                    on_client_event(event);
                    break;
                }
            }
        }
    });
}

void Host::stop()
{
    constexpr static char k_msg[] = "SHUTDOWN";

    write(m_pipe.out(), k_msg, sizeof(k_msg));
    m_thd.join();
}

void Host::on_server_event(og::Event &event)
{
    s32 socketfd;

    if (!event.is_readable()) {
        // Log error
        return;
    }

    if ((socketfd = m_listener->accept_handle()) == og::k_bad_socketfd) {
        // Log accept error
        return;
    }

    if (m_free_slots.empty()) {
        // Log client capacity reached- drop client
        og::intl::close(socketfd);
        return;
    }

    auto newid = m_free_slots.back();
    
    if (m_poll.add(socketfd, newid, og::Poll::e_read) < 0) {
        // Log monitor error- drop client
        og::intl::close(socketfd);
        return;
    }

    YKZ_GUEST_RESET(m_guests[newid]);
    m_guests[newid].socketfd = socketfd;
    m_free_slots.pop_back();

    // Log new client accepted
    std::cout << "Client accept, id: " << newid << "\n";
}

void Host::on_client_event(og::Event &event)
{

    auto id = event.id();

    if (event.is_error())
        goto error;

    if (event.is_readable()) {
        switch (data::rx_buffer(m_guests[id])) {
        case data::e_nothing:
            break;
        case data::e_made_progress:
        case data::e_all_done:
            if (!m_proto.okay(m_guests[id])) {
                if (YKZ_POLL_REFRESH(id, og::Poll::e_write) < 0)
                    goto error;
            } break;
        case data::e_connection_closed:
            goto drop;
        case data::e_error:
            goto error;
        }
    }

    if (event.is_writable()) {
        switch (data::tx_response(m_guests[id])) {
        case data::e_nothing:
        case data::e_made_progress:
            break;
        case data::e_all_done:
            if (YKZ_POLL_REFRESH(id, og::Poll::e_read) < 0)
                goto error;
            YKZ_GUEST_REFRESH(m_guests[id]);
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
    og::intl::close(m_guests[id].socketfd);
    m_free_slots.push_back(id);
}

} // namespace ykz
