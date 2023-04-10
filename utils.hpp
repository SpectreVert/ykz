/*
 * Created by Costa Bushnaq
 *
 * 21-01-2023 @ 17:01:12
 *
 * see LICENSE
*/

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdarg>

#include "ykz.hpp"

namespace ykz {

void log(char const*, ...);

s32 set_cloexec(s32, s32);
s32 set_nonblock(s32, s32);

void ipv4_to_string(u32, char* const);

s32 signal_wait();

enum io_result {
    IOR_AGAIN = 0,
    IOR_BUFFER_LIMIT,
    IOR_CONNECTION_CLOSED,
    IOR_ERROR,
};

io_result recv_header(Guest&);
io_result send_header(Guest&);
io_result send_file(Guest&);

} // namespace ykz

#endif /* UTILS_HPP*/
