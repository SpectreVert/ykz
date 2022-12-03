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

struct Guest_Info;

//! refresh after ending a request-response cycle
#define YKZ_CX_REFRESH(t_guest)\
    t_guest.header.reset();\
    t_guest.header_offset = 0;\
    t_guest.file.fd = 0;\
    t_guest.file.offset = 0;\
    t_guest.file.upper = 0;\
    t_guest.file.lower = 0;\

//! reset a connection (no close here)
#define YKZ_CX_RESET(t_guest)\
    t_guest.socketfd = og::k_bad_socketfd;\
    YKZ_CX_REFRESH(t_guest)

#define YKZ_LOG(...)\
    fprintf(stderr, __VA_ARGS__)

namespace data {

enum result : s32 {
    e_nothing = 0,
    e_made_progress,
    e_all_done,
    e_connection_closed,

    e_error = -1,
};

result rx_header(Guest_Info &guest);
result tx_response(Guest_Info &guest);
result tx_header(Guest_Info &guest);
result tx_file(Guest_Info &guest);

} // namespace data

} // namespace ykz

#endif /* UTILS_HPP_ */
