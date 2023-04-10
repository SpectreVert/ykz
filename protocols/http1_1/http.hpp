/*
 * Created by Costa Bushnaq
 *
 * 26-01-2023 @ 21:06:11
 *
 * see LICENSE
*/

#ifndef HTTP_HPP
#define HTTP_HPP

#include <climits>

#include "ykz.hpp"

namespace ykz {

namespace http1_1 {

constexpr char VERSION_PREFIX[]{ "HTTP/" };
constexpr char VERSION_NUMBER[]{ "1.1" };
constexpr char CRLF[]{ "\r\n" };

constexpr u32 METHOD_MAX{ 24 };
constexpr u32 FIELD_MAX{ 400 };

enum method {
    MTD_GET = 0,
    MTD_HEAD,

    NUM_METHODS,
};

enum req_field {
    REQ_HOST = 0,
    REQ_RANGE,
    REQ_IF_MODIFIED_SINCE,

    NUM_REQ_FIELDS
};

struct Request {
    method m;
    char path[PATH_MAX];
    char fields[NUM_REQ_FIELDS][FIELD_MAX];
};

enum status {
    S_OK = 200,
    S_PARTIAL_CONTENT = 206,

    S_NOT_MODIFIED = 304,

    S_BAD_REQUEST = 400,
    S_FORBIDDEN = 403,
    S_NOT_FOUND = 404,
    S_METHOD_NOT_ALLOWED = 405,
    S_REQUEST_TIMEOUT = 408,
    S_URI_TOO_LONG = 414,
    S_RANGE_NOT_SATISFIABLE = 416,
    S_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,

    S_INTERNAL_SERVER_ERROR = 500,
    S_HTTP_VERSION_NOT_SUPPORTED = 505,
};

enum res_field {
    RES_ACCEPT_RANGES = 0,
    RES_ALLOW,
    RES_CONTENT_LENGTH,
    RES_CONTENT_RANGE,
    RES_CONTENT_TYPE,
    RES_LAST_MODIFIED,
    RES_LOCATION,

    NUM_RES_FIELDS,
};

enum mime {
    MIM_DEFAULT = 0,
    MIM_C,
    MIM_H,
    MIM_HTML,
    MIM_HTM,
    MIM_XML,
    MIM_XHTML,
    MIM_CSS,
    MIM_TXT,
    MIM_MD,
    MIM_JPG,
    MIM_JPEG,
    MIM_PNG,
    MIM_GIF,
    MIM_MP3,
    MIM_FLAC,
    MIM_MP4,
    MIM_OGG,
    MIM_WEBM,
    MIM_PDF,
    MIM_GZ,
    MIM_TAR,
    MIM_ZIP,

    NUM_MIMES,
};

struct Response {
    status s;
    char path[PATH_MAX];
    char fields[NUM_RES_FIELDS][FIELD_MAX];
};

status extract_request(char const*, Request&);
status execute_request(Request&, Response&, Guest&);
bool   export_response(Response&, Guest&);

} // namespace http

} // namespace ykz

#endif /* HTTP_HPP */
