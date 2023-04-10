/*
 * Created by Costa Bushnaq
 *
 * 08-04-2023 @ 13:07:15
*/

#include "http.hpp"
#include "utils.hpp"

namespace ykz {

namespace http1_1 {

void init(void)
{
}

proto_result okay(void *data)
{
    Guest *guest = (Guest*)data;

    status   s;
    Request  rq;
    Response rs;

    /* read and execute request */
    if (s = extract_request(guest->header, rq), s == S_OK) {
        rs.s = execute_request(rq, rs, *guest);
    } else {
        rs.s = s;
    }

    /* prepare response */
    if (!export_response(rs, *guest)) {
        return PRS_INTERNAL_ERROR;
    }

    log("[#%d] [%d]: %s %s",
        guest->socket, rs.s,
        rq.m == MTD_GET ? "GET" : "HEAD", rs.path
    );

    return PRS_SWITCH_MODE;
}

void goodbye(void *data)
{
    (void) data;
}

void cleanup(void)
{
}

} // namespace http1_1

} // namespace ykz
