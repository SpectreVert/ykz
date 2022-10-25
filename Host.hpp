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
    StaticBuffer<YKZ_BUFFER_SIZE> buffer;
    u64 progress{0};

    data::type data_type{data::e_buffer};
    s32 fd{-1};
    // @Implement stream resource
};

// Host - server mainframe
//
struct Host {
    // @Cleanup(SpectreVert): 
    og::Poll m_poll; // this should be pertaining to each worker thread
    og::Events m_events; // ditto

    // This also doesn't need to be stored here as the FD is the only real
    // needed data that needs to be shared
    std::unique_ptr<og::TcpListener> m_listener{nullptr}; 

    Guest m_guests[YKZ_MAX_CLIENTS]; // move to thread
    std::vector<u32> m_free_slots; // same

    Protocol m_proto;
    std::thread m_thd;
    ipc::Pipe m_pipe;

    Host(Host const &) = delete;
    Host(Host &&) = delete;

    void start(og::SocketAddr &addr);
    void stop();

    void set_signal_handler() const;

    void on_server_event(og::Event &event);
    void on_client_event(og::Event &event);

}; // struct Host

} // namespace ykz

#endif /* YKZ_HOST_HPP_ */
