/*
 * Created by Costa Bushnaq
 *
 * 13-10-2022 @ 17:00:30
*/

#include "utils.hpp"
#include "Host.hpp"

#include <atomic>
#include <cassert>
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
    switch (info.d) {
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

} // namespace ykz
