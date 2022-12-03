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
result rx_header(Guest_Info &guest)
{
    auto socketfd  = guest.socketfd;
    auto &header   = guest.header;
    auto prev_size = header.size();
    s64  res;

    for (;;) {
        res = ::recv(socketfd, header.data(), header.vacant(), MSG_NOSIGNAL);
        if (res > 0) {
            header.cursor += res;
            if (header.size() == header.capacity)
                return e_all_done;
        } else {
            if (res == 0)
                return e_connection_closed;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            return e_error;
        }
    }
    if (prev_size < header.size())
        return e_made_progress;

    return e_nothing;
}

result tx_response(Guest_Info &guest)
{
    result r{e_nothing};

    if (guest.header.size() > 0) {
        r = tx_header(guest);
        if (r != e_all_done)
            return r;
    }

    if (1) {
        return tx_file(guest);
    }

    return r;
}

result tx_header(Guest_Info &guest)
{
    auto socketfd = guest.socketfd;
    auto header   = guest.header;
    auto prev_offset = guest.header_offset;
    s64 res;

    for (;;) {
        res = ::send(socketfd, header.view() + guest.header_offset,
                     header.size() - guest.header_offset, MSG_NOSIGNAL);
        if (res > 0) {
            guest.header_offset += res;
            if (guest.header_offset == header.size())
                return e_all_done;
        } else {
            if (res == 0)
                return e_connection_closed;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            return e_error;
        }
    }
    if (prev_offset > guest.header_offset)
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

// @Todo @Refactor In the long term we want to use upper/lower file
// ranges and implement a real tx_file function (not using sendfile)
result tx_file(Guest_Info &guest)
{
    auto socketfd = guest.socketfd;
    auto resourcefd = guest.file.fd;
    struct stat st;
    s64 res;

    std::memset(&st, 0, sizeof(st));
    if (fstat(resourcefd, &st)) {
        return e_error;
    }

    for (;;) {
        res = ::sendfile(socketfd, resourcefd, &guest.file.offset, st.st_size);
        if (res > 0 && guest.file.offset == st.st_size) {
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
