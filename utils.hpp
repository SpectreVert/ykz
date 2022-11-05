/*
 * Created by Costa Bushnaq
 *
 * 13-10-2022 @ 16:45:07
 *
 * see LICENSE
*/

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "buffers.hpp"

#include <functional>
#include <cstdio>

namespace ykz {

struct Guest;

//! Refresh the internal state of the buffer and the meta data.
#define YKZ_CX_REFRESH(t_guest)\
    t_guest.offset_buf = 0;\
    t_guest.offset_file = 0;\
    t_guest.buffer.reset();\
    t_guest.fpath[0] = '0'

//! Reset the internal state of `info`.
#define YKZ_CX_RESET(t_guest)\
    t_guest.socketfd = og::k_bad_socketfd;\
    YKZ_CX_REFRESH(t_guest)

#define YKZ_LOG(...)\
    fprintf(stderr, __VA_ARGS__)

namespace data {

enum type : s32 {
    e_buffer = 0,
    e_file,
    /* e_stream, @Implement */
};

// @FIXME(SV): e_nothing && e_made_progress can be unified
//  as e_again
enum result : s32 {
    e_nothing = 0,
    e_made_progress,
    e_all_done,
    e_connection_closed,

    e_error = -1,
};

result rx_buffer(Guest &info);
result tx_response(Guest &info);
result tx_buffer(Guest &info);
result tx_file(Guest &info);

} // namespace data

} // namespace ykz

#endif /* UTILS_HPP_ */
