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
#include <sys/sendfile.h>
#include <sys/stat.h>

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
    result r{e_nothing};

    if (info.buffer.size() > 0) {
        r = tx_buffer(info);
        if (r != e_all_done)
            return r;
    }

    if (std::strlen(info.fpath) > 0) {
        return tx_file(info);
    }

    return r;
}

result tx_buffer(Guest &info)
{
    auto socketfd = info.socketfd;
    auto buffer   = info.buffer;
    auto prev_offset = info.offset_buf;
    s64 res;

    for (;;) {
        res = ::send(socketfd, buffer.view() + info.offset_buf,
                     buffer.size() - info.offset_buf, MSG_NOSIGNAL);
        if (res > 0) {
            info.offset_buf += res;
            if (info.offset_buf == buffer.size())
                return e_all_done;
        } else {
            if (res == 0)
                return e_connection_closed;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            return e_error;
        }
    }
    if (prev_offset > info.offset_buf)
        return e_made_progress;

    return e_nothing; 
}

static bool file_open_st(char const *fpath, s32 &t_fd, struct stat &st)
{
    auto fd = open(fpath, O_RDONLY);
    if (fd < 0)
        return false;
    std::memset(&st, '0', sizeof(st));
    if (fstat(fd, &st) < 0) {
        close(fd);
        return false;
    }

    t_fd = fd;
    return true;
}

result tx_file(Guest &info)
{
    auto socketfd = info.socketfd;
    auto fpath = info.fpath;
    struct stat st;
    s64 res;
    s32 fd;

    if (!file_open_st(fpath, fd, st))
        return e_error;

    for (;;) {
        res = ::sendfile(socketfd, fd, &info.offset_file, st.st_size);
        if (res > 0 && info.offset_file == st.st_size) {
            return e_all_done;
        } else {
            if (res == 0)
                return e_connection_closed;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            return e_error;
        }
    }

    return e_nothing;
}

} // namespace data

} // namespace ykz
