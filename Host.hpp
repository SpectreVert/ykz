/*
 * Created by Costa Bushnaq
 *
 * 24-09-2022 @ 19:48:02
 *
 * see LICENSE
*/

#ifndef HOST_HPP_
#define HOST_HPP_

#include "Buffers.hpp"

#include "og/Poll.hpp"
#include "og/TcpListener.hpp"

#include <vector>

#include "ykz.config.hpp"

namespace ykz {

using og::s32;
using og::u32;

// Host - Default host class
//
struct Host {
    og::Poll m_poll;
    og::Events m_events;

    og::TcpListener *m_listener{nullptr};
    struct MetaInfo {
        enum {
            e_unknown,
            e_static_buffer,
            e_dynamic_buffer,
            e_file
        };

        void reset() {
            socketfd = og::k_bad_socketfd;
            refresh();
        }

        void refresh() {
            resource_type = e_unknown;
            response_progress = 0;
            buffer.reset();
        }

        s32 socketfd{og::k_bad_socketfd};
        u32 resource_type{e_unknown};
        u64 response_progress{0};
        StaticBuffer<YKZ_MSG_SIZE> buffer;
    } m_info[YKZ_MAX_CLIENTS];

    std::vector<u32> m_free_slots;
    
    virtual ~Host() = default;
    Host() = default;
    Host(Host const &) = delete;
    Host(Host &&) = delete;

    void start(); // ?
    void stop();  // ?

    void on_server_event(og::Event &event);
    void on_client_event(og::Event &event);

    // \return
    //     -1 : got an error on the socket
    //      0 : we made no progress
    //      1 : we made some progress
    //      2 : we received/sent everything we could
    s32 on_readable(u64 id);
    s32 on_writable(u64 id);

    bool is_request_valid(MetaInfo &info);

}; // struct Host

} // namespace ykz

#endif /* HOST_HPP_ */
