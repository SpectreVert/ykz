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
    s32 socketfd{og::k_bad_socketfd};

    // resource fields
    u64 offset_buf{0};
    s64 offset_file{0};

    StaticBuffer<BUFFER_SIZE> buffer;
    char fpath[PATH_MAX];
};

// Host - server mainframe
//
struct Host {
    bool is_started{false};
    og::TcpListener m_insock{};
    std::thread m_workers[NB_WORKERS];

    void start(og::SocketAddr &addr, Protocol *proto);
    void join();

}; // struct Host

} // namespace ykz

#endif /* YKZ_HOST_HPP_ */
