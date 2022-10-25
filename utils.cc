/*
 * Created by Costa Bushnaq
 *
 * 13-10-2022 @ 17:00:30
*/

#include "utils.hpp"
#include "Host.hpp"

#include <atomic>
#include <cassert>
#include <csignal>
#include <fcntl.h>
#include <stdio.h>

namespace ykz {

namespace data {

// @Improve Add s32 flags argument to this function
result rx_buffer(Guest &info)
{
    auto socketfd  = info.socketfd;
    auto &buffer   = info.buffer;
    auto prev_size = buffer.size();
    s64  res;

    for (;;) {
        res = ::recv(socketfd, buffer.data(), buffer.vacant(), MSG_NOSIGNAL);
        if (res > 0) {
            buffer.cursor += res;
            if (buffer.size() == buffer.capacity)
                return e_all_done;
        } else {
            if (res == 0)
                return e_connection_closed;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            return e_error;
        }
    }
    if (prev_size < buffer.size())
        return e_made_progress;

    return e_nothing;
}

result tx_response(Guest &info)
{
    switch (info.data_type) {
    case e_buffer:
        return tx_buffer(info);
    case e_file: // @Implement file sending
    default:
        // @Implement @Log error
        return e_nothing;
    }
}

result tx_buffer(Guest &info)
{
    auto socketfd = info.socketfd;
    auto buffer   = info.buffer;
    auto prev_progress = info.progress;
    s64 res;

    for (;;) {
        res = ::send(socketfd, buffer.view() + info.progress,
                     buffer.size() - info.progress, MSG_NOSIGNAL);
        if (res > 0) {
            info.progress += res;
            if (info.progress == buffer.size())
                return e_all_done;
        } else {
            if (res == 0)
                return e_connection_closed;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            return e_error;
        }
    }
    if (prev_progress > info.progress)
        return e_made_progress;

    return e_nothing; 
}

} // namespace data

namespace signal {

Filter::Filter(const std::initializer_list<s32> &signals) noexcept
    : m_set{}
{
    sigemptyset(&m_set);
    for (auto signum : signals) {
        sigaddset(&m_set, signum);
    }

    auto s = pthread_sigmask(SIG_BLOCK, &m_set, nullptr);
    assert(!s);
}

Filter::~Filter() noexcept
{
    pthread_sigmask(SIG_UNBLOCK, &m_set, nullptr);
}

s32 Filter::wait() const
{
    s32 signum{0};

    return sigwait(&m_set, &signum);
}

s32 Filter::wait_with_handler(std::function<bool(int)> handler) const
{
    while (1) {
        auto signum = wait();
        if (handler(signum))
            return signum;
    }
}

} // namespace signale

namespace ipc {

Pipe::Pipe()
{
    assert(!pipe2(fd, O_CLOEXEC | O_NONBLOCK));
}

Pipe::~Pipe()
{
    close(fd[e_in]);
    close(fd[e_out]);
}

s32 Pipe::in() const
{
    return fd[e_in];
}

s32 Pipe::out() const
{
    return fd[e_out];
}

} // namespace ipc

} // namespace ykz
