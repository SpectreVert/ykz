/*
 * Created by Costa Bushnaq
 *
 * 10-03-2023 @ 08:31:02
*/

#include <arpa/inet.h>
#include <cassert>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <grp.h>
#include <pwd.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>

#include "ykz.hpp"
#include "driver.hpp"
#include "utils.hpp"

namespace ykz {

static s32 host_make_socket(u32 address, u16 port)
{
    s32 socket;
    struct sockaddr_in addr_info{ 0 };
    socklen_t addr_len = sizeof(addr_info);

    /* create socket */
    socket = ::socket(PF_INET, SOCK_STREAM, 0);
    if (socket == -1) {
        log("socket:");
        return -1;
    }

    /* build the address struct */
    addr_info.sin_family = PF_INET;
    addr_info.sin_addr.s_addr = address;
    addr_info.sin_port = port;

    /* bind and listen */
    if (bind(socket, (sockaddr*)&addr_info, addr_len) < 0) {
        log("bind:");
        close(socket);
        return -1;
    }

    if (listen(socket, LISTEN_SOCKET_BACKLOG) < 0) {
        log("listen:");
        close(socket);
        return -1;
    }

    /* set nonblocking */
    if (set_nonblock(socket, 1) < 0) {
        log("ioctl:");
        close(socket);
        return -1;
    }

    return socket;
}

void host_start(Host &h, Protocol &p, u32 address, u16 port)
{
    char addr_str[INET_ADDRSTRLEN];
    struct passwd *user;
    struct group *group;

    /* DEFAULTS */
    char servedir[] = ".";
    char name[] = "nobody";
    char gang[] = "nogroup";

    memset(&h, 0, sizeof(Host));

    h.address = htonl(address);
    h.port    = htons(port);

    /* validate user */
    user = getpwnam(name);
    if (!user) {
        log("getpwnam:");
        return;
    }

    /* validate group */
    group = getgrnam(gang);
    if (!group) {
        log("getgrnam:");
        return;
    }

    /* create the listening socket (non-blocking) */
    h.socket = host_make_socket(h.address, h.port);
    if (h.socket == -1) { return; }

    /* ignore irrelevant signals */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        log("signal:");
        close(h.socket);
        return;
    }
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        log("signal:");
        close(h.socket);
        return;
    }

    /* chroot + chdir */
    if (chroot(servedir) < 0) {
        log("chroot:");
        log("chroot: You need to run as root or have CAP_SYS_CHROOT set");
        close(h.socket);
        return;
    }
    if (chdir("/") < 0) {
        log("chdir:");
        close(h.socket);
        return;
    }

    /* check that the target user is not privileged */
    if (user->pw_uid == 0 || group->gr_gid == 0) {
        log("Cannot run as privileged %s",
            (!user->pw_uid) ? (!group->gr_gid) ?
             "user and group":
             "user"          : "group");
        close(h.socket);
        return;
    }

    /* drop privileges */
    if (setgroups(1, &(group->gr_gid)) < 0) {
        log("setgroups:");
        log("setgroups: You need to run as root or have CAP_SETGID set");
        close(h.socket);
        return;
    }
    if (setgid(group->gr_gid) < 0) {
        log("setgid:");
        log("setgid: You need to run as root or have CAP_SETGID set");
        close(h.socket);
        return;
    }
    if (setuid(user->pw_uid) < 0) {
        log("setuid:");
        log("setuid: You need to run as root or have CAP_SETUID set");
        close(h.socket);
        return;
    }

    h.is_running = true;

    ipv4_to_string(h.address, addr_str);
    log("[%s:%hu] LISTEN", addr_str, port);

    for (u32 i = 0; i < DRIVERS_PER_HOST; i++) {
        h.workers[i] = std::thread(
            driver_thread,
            h.socket,
            std::ref(h.is_running),
            std::ref(p));
    }
}

void host_stop(Host &h)
{
    char addr_str[INET_ADDRSTRLEN];

    if (!h.is_running) {
        log("ykz: Host not running");
        return;
    }

    close(h.socket);
    h.is_running = false;

    ipv4_to_string(h.address, addr_str);
    log("\n[%s:%hu] CLOSING", addr_str, ntohs(h.port));

    for (u32 i = 0; i < DRIVERS_PER_HOST; i++) {
        h.workers[i].join();
    }

    log("[%s:%hu] STOPPED", addr_str, ntohs(h.port));
}

} // namespace ykz
