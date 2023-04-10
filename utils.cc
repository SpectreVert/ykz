/*
 * Created by Costa Bushnaq
 *
 * 21-01-2023 @ 17:00:26
*/

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <time.h>

#include "utils.hpp"

namespace ykz {

static void verr_log(char const *fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);

    if (fmt[strlen(fmt) - 1] == ':') {
        fputc(' ', stderr);
        perror(0x0);
    } else {
        fputc('\n', stderr);
    }
}

void log(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    verr_log(fmt, ap);
    va_end(ap);
}

s32 set_cloexec(s32 socket, s32 on)
{
    return ioctl(socket, on ? FIOCLEX : FIONCLEX);
}

s32 set_nonblock(s32 socket, s32 on)
{
    return ioctl(socket, FIONBIO, &on);
}

void ipv4_to_string(u32 address, char *const addr_str)
{
    struct in_addr addr_buf{ 0 };

    addr_buf.s_addr = address;
    inet_ntop(PF_INET, &addr_buf, addr_str, INET_ADDRSTRLEN);
}

s32 signal_wait()
{
    s32 status;
    sigset_t signal_set;

    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGHUP);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGQUIT);
    sigwait(&signal_set, &status);

    return status;
}

io_result recv_header(Guest &guest)
{
    auto socket = guest.socket;
    auto header = guest.header;
    auto &wcur  = guest.wcur;
    s64 res;

    do {
        res = recv(socket, header + wcur, HEADER_LENGTH - wcur, 0);
        if (res > 0) {
            wcur += res;
        } else if (res == 0) {
            return IOR_CONNECTION_CLOSED;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return IOR_AGAIN;
        } else {
            return IOR_ERROR;
        }
    } while (wcur < HEADER_LENGTH);

    return IOR_BUFFER_LIMIT;
}

io_result send_header(Guest &guest)
{
    auto socket = guest.socket;
    auto header = guest.header;
    auto &rcur  = guest.rcur;
    auto wcur   = guest.wcur;
    s64 res;

    do {
        res = send(socket, header + rcur, wcur - rcur, MSG_NOSIGNAL);
        if (res > 0) {
            rcur += res;
        } else if (res == 0) {
            return IOR_CONNECTION_CLOSED;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return IOR_AGAIN;
        } else {
            return IOR_ERROR;
        }
    } while (rcur < wcur);

    return IOR_BUFFER_LIMIT;
}

io_result send_file(Guest &guest)
{
    auto socket = guest.socket;
    auto &lower = guest.file.lower;
    auto upper  = guest.file.upper;
    s64 res;

    do {
        auto len = (u64)(upper - lower + 1);
        res = sendfile(socket, guest.file.fd, &lower, len);
        if (res > 0) {
            continue;
        } else if (res == 0) {
            return IOR_CONNECTION_CLOSED;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return IOR_AGAIN;
        } else {
            return IOR_ERROR;
        }
    } while (lower < upper);
    
    return IOR_BUFFER_LIMIT;
}

} // namespace ykz
