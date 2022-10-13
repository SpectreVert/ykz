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

void reset(Guest &info);
void refresh(Guest &info);

result rx_buffer(Guest &info);
result tx_response(Guest &info);
result tx_buffer(Guest &info);

} // namespace data

} // namespace ykz

#endif /* UTILS_HPP_ */
