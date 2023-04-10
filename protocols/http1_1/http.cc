/*
 * Created by Costa Bushnaq
 *
 * 05-02-2023 @ 19:39:08
*/

#define _XOPEN_SOURCE
#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "http.hpp"
#include "utils.hpp"

namespace ykz {

namespace http1_1 {

static char const *method_str[] = {
    "GET",
    "HEAD"
};

static char const *req_field_str[] = {
    "Host",
    "Range",
    "If-Modified-Since"
};

static char const *get_status_str(status s)
{
    switch (s) {
    case S_OK:
        return "OK";
    case S_PARTIAL_CONTENT:
        return "Partial Content";
    case S_NOT_MODIFIED:
        return "Not Modified";
    case S_BAD_REQUEST:
        return "Bad Request";
    case S_FORBIDDEN:
        return "Forbidden";
    case S_NOT_FOUND:
        return "Not Found";
    case S_METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case S_REQUEST_TIMEOUT:
        return "Request Time-out";
    case S_URI_TOO_LONG:
        return "URI Too Long";
    case S_RANGE_NOT_SATISFIABLE:
        return "Range Not Satisfiable";
    case S_REQUEST_HEADER_FIELDS_TOO_LARGE:
        return "Request Header Fields Too Large";
    case S_INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    case S_HTTP_VERSION_NOT_SUPPORTED:
        return "HTTP Version Not Supported";
    default:
        return "N/A";
    }
}

static char const *res_field_str[] = {
    "Accept-Ranges",
    "Allow",
    "Content-Length",
    "Content-Range",
    "Content-Type",
    "Last-Modified",
    "Location",
};

static char const *mime_exts[] = {
    "",
    "c",
    "h",
    "html",
    "htm",
    "xml",
    "xhtml",
    "css",
    "txt",
    "md",
    "jpg",
    "jpeg",
    "png",
    "gif",
    "mp3",
    "flac",
    "mp4",
    "ogg",
    "webm",
    "pdf",
    "gz",
    "tar",
    "zip"
};

static char const *mime_encs[] = {
    "application/octet-stream",
    "text/plain; charset=utf-8",
    "text/plain; charset=utf-8",
    "text/html; charset=utf-8",
    "text/html; charset=utf-8",
    "application/xml; charset=utf-8",
    "application/xhtml+xml; charset=utf-8",
    "text/css; charset=utf-8",
    "text/plain; charset=utf-8",
    "text/plain; charset=utf-8",
    "image/jpg",
    "image/jpg",
    "image/png",
    "image/gif",
    "audio/mpeg",
    "audio/flac",
    "video/mp4",
    "video/ogg",
    "video/webm",
    "application/x-pdf",
    "application/x-gtar",
    "application/tar",
    "application/zip",
};

static s32 do_snprintf(char *buf, u64 buf_len, char const *fmt, ...)
{
    va_list args;
    s32 ret;

    va_start(args, fmt);
    ret = vsnprintf(buf, buf_len, fmt, args);
    va_end(args);

    return !(ret >= 0 && (u64)ret < buf_len);
}

static void decode(char const src[PATH_MAX], char dest[PATH_MAX])
{
    char const *s = src;
    u64 i = 0;
    u8 r;

    for (; *s; s++, i++) {
        if (*s == '%' && (sscanf(s + 1, "%2hhx", &r) == 1)) {
            dest[i] = r;
            s += 2;
        } else {
            dest[i] = *s;
        }
    }
    dest[i] = 0;
}

static void encode(char const src[PATH_MAX], char dest[PATH_MAX])
{
    char const *s = src;
    u64 i = 0;

    for (; *s && i < (PATH_MAX - 4); s++) {
        if (iscntrl(*s) || (u8)*s > 127) {
            i += snprintf(dest + i, PATH_MAX - i, "%%%02X", (u8)*s);
        } else {
            dest[i] = *s;
            i++;
        }
    }
    dest[i] = 0;
}

static status validate_range(char *s, Guest::File &file)
{
    assert(file.size > 0);
    char const *e, *l, *r;
    char *f;

    /* default to the complete range */
    file.lower = 0;
    file.upper = file.size - 1;

    /* if there is no range, return S_OK to continue */
    if (!s || !s[0]) {
        return S_OK;
    }

    /* required statement */
    if (strncmp(s, "bytes=", sizeof("bytes=") - 1)) {
        return S_BAD_REQUEST;
    }
    l = s + sizeof("bytes=") - 1;

    /*
        bytes=first-last\0
              ^     ^   ^
              l     r   e
    */

    /*
        @Note for simplicity and convenience we don't accept multipart
        ranges. We accept ranges that contain exactly one hypen and
        reject the others.
    */

    /* traverse the rest of the header field */
    for (e = l, r = 0x0; *e != 0; e++) {
        if (*e < '0' || *e > '9') {
            if (*e == '-') {
                /* check if we have no left element */
                if (l == e) { s[sizeof("bytes=") - 1] = 0; }
                /* ensure we didn't find a hyphen previously */
                if (r) { return S_BAD_REQUEST; }
                /* mark the right element */
                else { r = e + 1; }
            } else if (*e == ',') {
                /* maybe we can troll them and send 200 OK instead */
                return S_RANGE_NOT_SATISFIABLE;
            } else {
                return S_BAD_REQUEST;
            }
        }
    }

    if (!r) {
        /* we didn't find any hyphen */
        return S_BAD_REQUEST;
    }
    
    if (l[0]) {
        /* range has a left element */

        /* we set the hyphen to '\0' so we can send the string to strtoll */
        *strchr(s, '-') = 0;

        /* as per strtoll(3) we set errno to 0 */
        errno = 0;
        file.lower = strtoll(l, &f, 10);
        if (errno || f == l || *f != 0) { return S_BAD_REQUEST; }

        if (r[0]) {
            /* range also has right element */
            errno = 0;
            file.upper = strtoll(r, &f, 10);
            if (errno || f == r || *f != 0) { return S_BAD_REQUEST; }
        }

        if (file.lower >= file.size || file.lower > file.upper) {
            return S_RANGE_NOT_SATISFIABLE;
        }

        /* file.upper can't go over file.size */
        if (file.upper >= file.size) { file.upper = file.size - 1; }
    } else {
        /* range only has a right element */

        /* alas, we hoped */
        if (!r[0]) {
            return S_BAD_REQUEST;
        }

        errno = 0;
        file.lower = strtoll(r, &f, 10);
        if (errno || f == r || *f != 0) { return S_BAD_REQUEST; }
    
        /* verify we don't offset less than how many bytes we have */
        /* then invert to the real offset */
        if (file.lower > file.size) { file.lower = 0; }
        else { file.lower = file.size - file.lower; }

        /* file.upper is already set to the maximum range file.size - 1 */
    }

    return S_PARTIAL_CONTENT;
}

status extract_request(char const *header, Request &req)
{
    s32 i;
    u64 mlen;
    char const *p, *u, *v;
    char *c;

    memset(&req, 0, sizeof(req));

    /* METHOD */
    for (i = 0; i < NUM_METHODS; i++) {
        mlen = strlen(method_str[i]);
        if (!strncmp(method_str[i], header, mlen)) {
            req.m = static_cast<method>(i);
            break;
        }
    }

    if (i == NUM_METHODS) {
        return S_METHOD_NOT_ALLOWED;
    }

    if (header[mlen] != ' ') {
        return S_BAD_REQUEST;
    }

    /* URI PATH */
    u = header + mlen + 1;

    if (v = strchr(u, ' '), !v) {
        return S_BAD_REQUEST;
    }

    if ((u64)(v - u + 1) > sizeof(req.path)) {
        return S_URI_TOO_LONG;
    }
    memcpy(req.path, u, v - u);
    req.path[v - u] = 0;
    decode(req.path, req.path);

    p = v + 1;

    /* HTTP VERSION */
    if (strncmp(p, VERSION_PREFIX, sizeof(VERSION_PREFIX) - 1)) {
        return S_BAD_REQUEST;
    }
    p += sizeof(VERSION_PREFIX) - 1;

    if (strncmp(p, VERSION_NUMBER, sizeof(VERSION_NUMBER) - 1)) {
        return S_HTTP_VERSION_NOT_SUPPORTED;
    }
    p += sizeof(VERSION_NUMBER) - 1;

    if (strncmp(p, CRLF, sizeof(CRLF) - 1)) {
        return S_BAD_REQUEST;
    }
    p += sizeof(CRLF) - 1;

    /* @Note we ignore query and fragment for simplicity */

    /* FIELDS */
    while (*p) {
        /* <field_name> : <ws> <field_content> */

        for (i = 0; i < NUM_REQ_FIELDS; i++) {
            auto &field = req_field_str[i];
            if (!strncasecmp(p, field, strlen(field))) {
                break;
            }
        }

        if (i == NUM_REQ_FIELDS) {
            /* no field found, we skip the line and try to continue */
            if (v = strstr(p, CRLF), !v) {
                return S_BAD_REQUEST;
            }
            p = v + sizeof(CRLF) - 1;
            continue;
        }

        p += strlen(req_field_str[i]);

        /* there should be a colon */
        if (*p != ':') {
            return S_BAD_REQUEST;
        }

        /* bye bye whitespaces */
        for (++p; *p == ' ' || *p == '\t'; p++);

        /* find the field content */
        if (v = strstr(p, CRLF), !v) {
            return S_BAD_REQUEST;
        }

        if ((u64)(v - p + 1) > sizeof(req.fields[i])) {
            return S_REQUEST_HEADER_FIELDS_TOO_LARGE;
        }

        /* copy the field content */
        memcpy(req.fields[i], p, v - p);
        req.fields[i][v - p] = 0;

        /* go to next line */
        p = v + sizeof(CRLF) - 1;
    }

    /* remove port suffix */
    if ((c = strchr(req.fields[REQ_HOST], ':'))) {
        /* check that it's not empty */
        if ((*c + 1) == 0) {
            return S_BAD_REQUEST;
        }
        *c = 0;
    }

    return S_OK;
}

status execute_request(Request &req, Response &res, Guest &guest)
{
    struct stat fs;
    struct tm ft;

    /* set response to zero and copy the URI */
    memset(&res, 0, sizeof(Response));
    memcpy(res.path, req.path, sizeof(Response::path));

    /* check for leading '/' */
    if (res.path[0] != '/') {
        return S_BAD_REQUEST;
    }

    /* reject all non-known hidden targets */
    if (strstr(res.path, "/.") && strcmp(res.path, "/.well-known/")) {
        return S_FORBIDDEN;
    }

    /* stat the target */
    if (stat(res.path, &fs) < 0) {
        if (errno == EACCES) { return S_FORBIDDEN; }
        else                 { return S_NOT_FOUND; }
    }

    /* directory listing not supported yet */
    if (S_ISDIR(fs.st_mode)) {
        return S_BAD_REQUEST;
    }

    /* check if the file is readable */
    if (access(res.path, R_OK)) {
        return S_FORBIDDEN;
    }

    /* extract If-Modified-Since timestamp */
    if (req.fields[REQ_IF_MODIFIED_SINCE][0]) {
        if (!strptime(req.fields[REQ_IF_MODIFIED_SINCE],
                      "%a, %d %b %Y %T GMT", &ft)) {
            return S_BAD_REQUEST;
        }

        /* compare with last modification date of file */
        if (difftime(fs.st_mtim.tv_sec, timegm(&ft)) <= 0) {
            return S_NOT_MODIFIED;
        }
    }

    /* @Todo: improve with a cache */
    /* open the file */
    guest.file.fd = open(res.path, O_RDONLY);
    if (guest.file.fd == -1) {
        return S_INTERNAL_SERVER_ERROR;
    }

    /* indicate we have a file */
    guest.file.is_pending = true;

    /* store the file size */
    guest.file.size = fs.st_size;

    /* store the file's last modification time */
    guest.file.last_modified = fs.st_mtim.tv_sec;

    /* extract Range bytes if any */
    return validate_range(req.fields[REQ_RANGE], guest.file);
}

bool export_response(Response &res, Guest &guest)
{
    struct tm ft;
    s32 r, mime = 0;
    char *p = 0x0;
    char time_buf[FIELD_MAX];
    s64 time_stmp;

    /* fill optional and required header fields */
    /* Allow (opt) */
    if (res.s == S_METHOD_NOT_ALLOWED) {
        if (do_snprintf(res.fields[RES_ALLOW], FIELD_MAX, "Allow: GET, HEAD")) {
            res.s = S_INTERNAL_SERVER_ERROR;
            goto finish;
        }
    }

    /* Accept-Ranges */
    if (do_snprintf(res.fields[RES_ACCEPT_RANGES], FIELD_MAX, "bytes")) {
        res.s = S_INTERNAL_SERVER_ERROR;
        goto finish;
    }

    /* if we have no error and a file to send... */
    if (res.s < 300 && guest.file.is_pending) {

        /* Content-Length */
        if (do_snprintf(res.fields[RES_CONTENT_LENGTH], FIELD_MAX,
            "%zd", guest.file.upper - guest.file.lower + 1)) {
            res.s = S_INTERNAL_SERVER_ERROR;
            goto finish;
        }

        /* Content-Range */
        if (res.s == S_RANGE_NOT_SATISFIABLE) {
            if (do_snprintf(
                res.fields[RES_CONTENT_RANGE], FIELD_MAX,
                "bytes */%zd", guest.file.size)) {
                res.s = S_INTERNAL_SERVER_ERROR;
                goto finish;
            }
        } else if (res.s == S_PARTIAL_CONTENT) {
            if (do_snprintf(res.fields[RES_CONTENT_RANGE], FIELD_MAX,
                "bytes %zd-%zd/%zd",
                guest.file.lower, guest.file.upper, guest.file.size)) {
                res.s = S_INTERNAL_SERVER_ERROR;
                goto finish;
            }
        }

        /* Content-Type */
        p = strrchr(res.path, '.');
        if (p) {
            for (mime = 1; mime < NUM_MIMES; mime++) {
                if (!strcmp(mime_exts[mime], p + 1)) {
                    break;
                }
            }
        }
        if (mime == NUM_MIMES) { mime = 0; }

        if (do_snprintf(res.fields[RES_CONTENT_TYPE], FIELD_MAX,
            "%s", mime_encs[mime])) {
            res.s = S_INTERNAL_SERVER_ERROR;
            goto finish;
        }

        /* Last-Modified */
        if (gmtime_r(&guest.file.last_modified, &ft) == 0x0 ||
            strftime(res.fields[RES_LAST_MODIFIED], FIELD_MAX,
                     "%a, %d %b %Y %T GMT", &ft) == 0) {
            res.s = S_INTERNAL_SERVER_ERROR;
            goto finish;
        }
    }


finish:
    time_stmp = time(0x0);
    if (gmtime_r(&time_stmp, &ft) == 0x0 ||
        strftime(time_buf, FIELD_MAX, "%a, %d %b %Y %T GMT", &ft) == 0) {
        return false;
    }

    /* reset cursors */
    guest.rcur = guest.wcur = 0;

    /* status line + mandatory headers */
    r = snprintf(guest.header + guest.wcur, sizeof(guest.header) - guest.wcur,
        "HTTP/1.1 %u %s\r\n"
        "Connection: %s\r\n" /* @Note: the server has a hard bypass on this */
        "Date: %s\r\n",
        res.s, get_status_str(res.s), 
        res.s < 300 ? "keep-alive" : "close", time_buf);
    if (r < 0 || (u64)r >= sizeof(guest.header) + guest.wcur) { return false; }
    else { guest.wcur += r; }

    /* headers */
    for (u32 i = 0; i < NUM_RES_FIELDS; i++) {
        if (res.fields[i][0]) {
            r = snprintf(guest.header + guest.wcur,
                sizeof(guest.header) - guest.wcur,
                "%s: %s\r\n", res_field_str[i], res.fields[i]);
            if (r < 0 || (u64)r >= sizeof(guest.header) + guest.wcur)
            { return false; }
            else { guest.wcur += r; }
        }
    }

    /* separator */
    r = snprintf(guest.header + guest.wcur, sizeof(guest.header) - guest.wcur, CRLF);
    if (r < 0 || (u64)r >= sizeof(guest.header) + guest.wcur) { return false; }
    else { guest.wcur += r; }

    return res.s < 300 ? true : false;
}

} // namespace http1_1

} // namespace ykz
