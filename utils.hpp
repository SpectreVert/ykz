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

namespace ykz {

struct Guest;

#define YKZ_POLL_REFRESH(t_id, t_interest)\
    m_poll.refresh(m_guests[t_id].socketfd, id, t_interest)

//! Refresh the internal state of the buffer and the meta data.
#define YKZ_GUEST_REFRESH(t_guest)\
    t_guest.buffer.reset();\
    t_guest.progress = 0;\
    t_guest.data_type = data::e_NA;

//! Reset the internal state of `info`.
#define YKZ_GUEST_RESET(t_guest)\
    t_guest.socketfd = og::k_bad_socketfd;\
    YKZ_GUEST_REFRESH(t_guest)

namespace data {

enum type : s32 {
    e_NA = 0,
    e_buffer,
    e_file,
    /* e_stream, @Implement */
};

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

} // namespace data

} // namespace ykz

#endif /* UTILS_HPP_ */
