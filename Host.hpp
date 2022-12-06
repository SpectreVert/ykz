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

// Guest_Info - visitor portfolio
//
struct Guest_Info {
    enum state {
        VACANT = 0,
        RX_MODE,
        TX_MODE,
    };
    
    s32 socketfd;

    StaticBuffer<HEADER_SIZE> header;
    u64 header_offset;

    bool send_file;
    struct {
        s32 fd;
        s64 offset;
        u64 upper;
        u64 lower;
    } file;
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
