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

#include "og/Poll.hpp"
#include "og/TcpListener.hpp"

#include <vector>

#include "./ykz.config.hpp"

namespace ykz {

using og::s32;
using og::u32;

// Guest - visitor portfolio
//
struct Guest {
    s32 socketfd{og::k_bad_socketfd};
    StaticBuffer<YKZ_BUFFER_SIZE> buffer;
    u64 progress{0};

    s32 data_type{0};
    s32 fd{-1};
    // @Implement stream resource
};

// Host - server mainframe
//
struct Host {
    og::Poll m_poll;
    og::Events m_events;

    og::TcpListener *m_listener{nullptr};
    Guest m_guests[YKZ_MAX_CLIENTS];
    std::vector<u32> m_free_slots;
    
    virtual ~Host() = default;
    Host() = default;
    Host(Host const &) = delete;
    Host(Host &&) = delete;

    void start(); // ?
    void stop();  // ?

    void on_server_event(og::Event &event);
    void on_client_event(og::Event &event);

}; // struct Host

} // namespace ykz

#endif /* YKZ_HOST_HPP_ */
