/*
 * Created by Costa Bushnaq
 *
 * 26-06-2022 @ 09:44:51
*/

#include <iostream>

#include "Host.hpp"

int main(int ac, char *av[])
{
    ykz::Host h;

    h.start();

    // ykz::Slab<std::string, 15> slab;
    // std::cout << slab.insert(std::string("Grosse farce")) << '\n';
    // std::cout << slab.insert(std::string("Second farce")) << '\n';

    // std::cout << slab.remove(13) << '\n';

    // std::cout << slab.insert(std::string("Troisième farce")) << '\n';
    // std::cout << slab.size() << '\n';

    // ykz::RingBuffer<test, 15> buf;
    // std::cout << "YKZ initiated\n";

    // buf.push_front().str = "New string";

    // buf.push_front().str = "Hello Sailor";

    // std::cout << buf.in_cursor << '\n';
    // std::cout << buf.out_cursor << '\n';

    // buf.pop_back();
    // buf.pop_back();
    // buf.pop_back();

    // std::cout << buf.in_cursor << '\n';
    // std::cout << buf.out_cursor << '\n';

    // for (int i = 0; i < 20; i++) {
    //     buf.push_front().str = "New string";
    // }

    // std::cout << buf.in_cursor << '\n';
    // std::cout << buf.out_cursor << '\n';

    // std::cout << "=====================\n";

    // og::TcpListener listener;
    // og::SocketAddrV4 addr(og::Ipv4(127, 0, 0, 1), 6970);
    // if (listener.bind(addr) < 0) {
    //     std::cout << "err on bind\n";
    //     return 0;
    // }
    // if (listener.listen(128) < 0) {
    //     std::cout << "err on listen\n";
    //     return 0;
    // }

    // ykz::Worker w(&listener);

    // std::cout << "Listning on " << addr.ip_host_order().to_string() << ":" << addr.port_host_order() << '\n';
    // w.poll_loop();

    // return 0;
}
