/*
 * Created by Costa Bushnaq
 *
 * 26-10-2022 @ 15:47:43
 *
 * see LICENSE
*/

#ifndef HTTP_HPP_
#define HTTP_HPP_

#include "Protocol.hpp"

#include "Host.hpp"

#include <climits>
#include <unistd.h>

namespace ykz {

struct Http : Protocol {
    Http() {
        init = []() -> s32 {
            return 0;
        };

        okay = internal_okay;

        cleanup = []() -> s32 {
            return 0;
        };
    };


    /* Under this is HTTP stuff */

#define k_ver_prefix "HTTP/"
#define k_ver_number "1.1"
#define k_CR "\r\n"
#define FIELD_MAX 400
#define METHOD_MAX 24

    using buffer_t = StaticBuffer<BUFFER_SIZE>;

    struct Request {
        enum method {
            GET,

            NUM_METHODS,
        };
        enum field {
            HOST,

            NUM_REQ_FIELDS,
        };

        method m;
        char path[PATH_MAX] = {};
        char const fields[NUM_REQ_FIELDS][FIELD_MAX] = {};
    };

    struct Response {
        enum status {
            OK = 200,
            BAD_REQUEST = 400,
            METHOD_NOT_ALLOWED = 405,
            REQUEST_URI_TOO_LARGE = 414,
            VERSION_NOT_SUPPORTED = 505,

            NUM_STATUS,
        };
        enum field {
            CONTENT_LENGTH,

            NUM_RES_FIELDS,
        };

        status s{BAD_REQUEST};
        char path[PATH_MAX] = {};
        char const fields[NUM_RES_FIELDS][FIELD_MAX] = {};
    };

    static Response::status
    parse_request_line(StaticBuffer<BUFFER_SIZE> &header, Request &request)
    {
        s32 i;
        u64 len;
        char const *p, *u, *v;

        static constexpr char method_str[Request::NUM_METHODS][METHOD_MAX] {
            "GET"
        };

        for (i = 0; i < Request::NUM_METHODS; i++) {
            len = std::strlen(method_str[i]);
            if (!std::strncmp(method_str[i], header.view(), len)) {
                request.m = static_cast<Request::method>(i);
                break;
            }
        }

        if (i == Request::NUM_METHODS) {
            return Response::METHOD_NOT_ALLOWED;
        }

        if (header.view()[len] != ' ') {
            return Response::BAD_REQUEST;
        }

        // set cursor to start of URI
        u = header.view() + len + 1;

        if (!(v = std::strchr(u, ' '))) { return Response::BAD_REQUEST; }
        v += 1;
        
        if ((len = (u64)(v - u - 1)) > sizeof(Request::path)) {
            return Response::REQUEST_URI_TOO_LARGE;
        }
        std::memset(request.path, 0, sizeof(Request::path));
        std::strncpy(request.path, u, (u64)(v - u - 1)); 

        // check HTTP version
        p = v;
        if (std::strncmp(p, k_ver_prefix, sizeof(k_ver_prefix) - 1)) {
            return Response::BAD_REQUEST;
        }
        p += sizeof(k_ver_prefix) - 1;

        if (std::strncmp(p, k_ver_number, sizeof(k_ver_number) - 1)) {
            return Response::VERSION_NOT_SUPPORTED;
        }
        p += sizeof(k_ver_number) - 1;

        return Response::OK;
    }

    s32 static
    internal_okay(Guest &info)
    {
        auto &buffer = info.buffer;
        Request request;
        Response response;

        if ((response.s = parse_request_line(buffer, request)) != Response::OK) {
            goto make_header;
        }

        std::strcpy(info.fpath, request.path);

make_header:
        /* When we arrive here we have completely parsed the request;
         * we reuse the buffer to write the response header. */

        buffer.reset();
        auto r = std::snprintf(buffer.data(), buffer.vacant(),
            "%s%s %i %s" k_CR,
            k_ver_prefix,
            k_ver_number,
            response.s,
            status_to_str(response.s)
        );

        assert(r > 0);
        if (static_cast<u64>(r) > buffer.capacity) {
            buffer.cursor += buffer.capacity;
        } else {
            buffer.cursor += r;
        }

        return 0;
    }

    /* Miscallaneous functions */

    static const char*
    status_to_str(Response::status s)
    {
        switch (s) {
        case Response::OK:
            return "OK";
        case Response::BAD_REQUEST:
            return "Bad Request";
        case Response::METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case Response::REQUEST_URI_TOO_LARGE:
            return "Request-URI Too Large";
        case Response::VERSION_NOT_SUPPORTED:
            return "HTTP Version not supported";
        default:
            assert(0);
        }
    }

}; // struct Http

} // namespace ykz

#endif /* HTTP_HPP_ */
