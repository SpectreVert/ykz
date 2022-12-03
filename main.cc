/*
 * Created by Costa Bushnaq
 *
 * 26-06-2022 @ 09:44:51
*/

#include <iostream>
#include <csignal>

#include "Host.hpp"

#include "modules/http_mini.hpp"

#include "utils.hpp"

using namespace ykz;

void sigcleanup(int sig)
{
    kill(0, sig);
    _exit(1);
}

void handle_signals(const std::initializer_list<s32> sigs, void(*handler)(int))
{
    struct sigaction sa; std::memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    for (auto sig : sigs) {
        sigaction(sig, &sa, 0x0);
    }
}

int main(int ac, char *av[])
{
    auto proto = http::mini::make_protocol();
    Host h;
    og::SocketAddr addr(
        og::Ipv4(127, 0, 0, 1), 
        6970
    );


    handle_signals({SIGTERM, SIGHUP, SIGINT, SIGQUIT, SIGCHLD}, &sigcleanup);

    h.start(addr, proto);
    h.join();
}
