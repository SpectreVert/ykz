/*
 * Created by Costa Bushnaq
 *
 * 04-10-2022 @ 17:19:41
*/

#include "Host.hpp"

#include "og/TcpStream.hpp"

#include <iostream>

using namespace ykz;

void Host::start()
{
    og::SocketAddrV4 addr(og::Ipv4("127.0.0.1"), 6970);

    m_listener = new og::TcpListener();

    if (m_listener->bind(addr) < 0) {
        std::cout << "bind error\n";
        return;
    }

    if (m_listener->listen(SOMAXCONN) < 0) {
        std::cout << "listen error\n";
        return;
    }

    std::cout << "serving 127.0.0.1:6970\n";

    for (u32 i = 0; i < YKZ_MAX_CLIENTS; i++) {
        m_free_slots.push_back(i);
    }

    if (m_poll.add(m_listener->handle(), 256, og::Poll::e_read|og::Poll::e_shared) < 0) {
        std::cout << "poll add error\n";
        return;
    }

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
            default:
                on_client_event(event);
                break;
            }
        }
    }
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

    m_free_slots.pop_back();
    m_info[newid].reset();
    m_info[newid].socketfd = socketfd;
    // Log new client accepted
    std::cout << "Client accept, id: " << newid << "\n";
}

void Host::on_client_event(og::Event &event)
{
    auto id = event.id();
    auto socketfd = m_info[id].socketfd;

    if (event.is_error())
        goto error;

    if (event.is_readable()) {
        switch (on_readable(id)) {
        case 2:
        case 1:
            if (is_request_valid(m_info[id])) {
                if (m_poll.refresh(socketfd, id, og::Poll::e_write) < 0)
                    goto error;
                // TODO: process request here
                break;
            }
            // We received something but it's still not done.
            break;
        case 0:
            break;
        case -1:
            goto error;
        case -2:
            goto drop;
        }
    }

    if (event.is_writable()) {
        switch (on_writable(id)) {
        case 2:
            if (m_poll.refresh(socketfd, id, og::Poll::e_read) < 0)
                goto error;
            m_info[id].refresh();
            break;
        case 1:
        case 0:
            // Nothing changed we're waiting on more data
            break;
        case -1:
            goto error;
        case -2:
            goto drop;
        }
    }

    return;

error:
    std::cout << "got error\n";
drop:
    og::intl::close(socketfd);
    m_free_slots.push_back(id);
}

s32 Host::on_readable(u64 id)
{
    auto &info = m_info[id];
    auto &buf = info.buffer;
    auto prev_size = buf.size();
    s64 res;

    for (;;) {
        res = ::recv(info.socketfd, buf.data(), buf.vacant(), MSG_NOSIGNAL);
        if (res > 0) {
            buf.cursor += res;
            if (buf.size() == buf.capacity)
                return 2; // Buffer is full
        } else {
            if (res == 0) // Connection closed
                return -2;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            return -1; // We got an error
        }
    }

    if (prev_size < buf.size()) // We got some new bytes
        return 1;

    // We get nothing new
    return 0;
}

s32 Host::on_writable(u64 id)
{
    auto &info = m_info[id];
    auto &buf = info.buffer;
    auto prev_progress = info.response_progress;
    s64 res;

    assert(info.resource_type != MetaInfo::e_unknown);

    switch (info.resource_type) {
    case MetaInfo::e_static_buffer:
        for (;;) {
            res = ::send(info.socketfd, buf.view() + info.response_progress,
                         buf.size() - info.response_progress, MSG_NOSIGNAL);
            if (res > 0) {
                info.response_progress += res;
                if (info.response_progress == buf.size())
                    return 2; // We sent everything we have to sent
            } else {
                if (res == 0) // Connection closed
                    return -2;
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                    break; // We sent some bytes but not everything
                return -1; // We got an error
            }
        };
        
        if (prev_progress < info.response_progress) // We sent some new bytes
            return 1;

        // We sent nothing new
        return 0;

    case MetaInfo::e_dynamic_buffer:
    case MetaInfo::e_file:
    default:
        return -1;
    };
}

bool Host::is_request_valid(MetaInfo &info)
{
    info.resource_type = MetaInfo::e_static_buffer;

    return true;
}
