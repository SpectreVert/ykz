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

#define YKZ_POLL_REFRESH(t_id, t_interest)\
    m_poll.refresh(m_guests[t_id].socketfd, id, t_interest)

//! Refresh the internal state of the buffer and the meta data.
#define YKZ_GUEST_REFRESH(t_guest)\
    t_guest.buffer.reset();\
    t_guest.progress = 0;\
    t_guest.data_type = data::e_buffer;

//! Reset the internal state of `info`.
#define YKZ_GUEST_RESET(t_guest)\
    t_guest.socketfd = og::k_bad_socketfd;\
    YKZ_GUEST_REFRESH(t_guest)

#define YKZ_SIGNAL_HANDLER_SET_CHANNEL(t_fd)\
    signal::set_handler([](s32 signal) { (void) signal;\
        constexpr char k_msg[] = "SHUTDOWN";\
        write(signal::the_fx_fd, k_msg, sizeof(k_msg));\
    });\
    signal::the_fx_fd = t_fd;

#define YKZ_LOG(...)\
    fprintf(stderr, __VA_ARGS__)

namespace data {

enum type : s32 {
    e_buffer = 0,
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

namespace signal {

static s32 the_fx_fd{0};

struct Filter {
    sigset_t m_set;

    explicit Filter(const std::initializer_list<s32> &signals) noexcept;
    ~Filter() noexcept;

    Filter(Filter const&) = delete;
    Filter(Filter &&) = delete;

    s32 wait() const;
    s32 wait_with_handler(std::function<bool(s32)> handler) const;

}; // struct Filter

} // namespace signal

namespace ipc {

struct Pipe {
    enum { e_in = 0, e_out = 1 };
    s32 fd[2] = {0, 0};

    Pipe();
    virtual ~Pipe();

    s32 in() const;
    s32 out() const;
};

} // namespace ipc

} // namespace ykz

#endif /* UTILS_HPP_ */
