/*
 * Created by Costa Bushnaq
 *
 * 24-09-2022 @ 19:48:02
 *
 * see LICENSE
*/

#ifndef YKZ_HOST_HPP_
#define YKZ_HOST_HPP_

#include "buffers.hpp"
#include "Protocol.hpp"

#include "og/defs.hpp"
#include "og/Poll.hpp"
#include "og/TcpListener.hpp"

#include <climits>
#include <vector>
#include <memory>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>

#include "./ykz.config.hpp"

namespace ykz {

using og::u32;
using og::s64;

// Guest - visitor portfolio
//
struct Guest {
    static constexpr auto k_bad_file{og::k_bad_socketfd};

    s32 socketfd{og::k_bad_socketfd};

    enum {
        VACANT,
        RX_HEADER,
        TX_HEADER,
        TX_FILE,

    } s{VACANT};

    StaticBuffer<BUFFER_SIZE> header;
    u64 offset_header{0};

    s32 resourcefd{k_bad_file};
    s64 offset_resource{0};
};

// Host - server mainframe
//
struct Host {
    // FIXME(SV): transform to atomic<bool>
    bool is_started{false};
    og::TcpListener m_insock{};
    std::thread m_workers[NB_WORKERS];

    void start(og::SocketAddr &addr, Protocol *proto);
    void join();

}; // struct Host

} // namespace ykz

#endif /* YKZ_HOST_HPP_ */
