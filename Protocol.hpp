/*
 * Created by Costa Bushnaq
 *
 * 13-10-2022 @ 19:35:22
 *
 * see LICENSE
*/

#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include "utils.hpp"

#include "og/defs.hpp"

#include <functional>

#include <iostream>

namespace ykz {

using og::s32;

struct Guest;

struct Protocol {
    typedef std::function<s32()> init_fn;
    typedef std::function<s32(Guest&)> okay_fn;
    typedef std::function<s32()> cleanup_fn;

    init_fn init = []() -> s32 {
        std::cout << "Initialized Module\n";
        return 0;
    };
    okay_fn okay = [](Guest &info) -> s32 { (void) info;
        std::cout << "Hello, Sailor!\n";
        return 0;
    };
    cleanup_fn cleanup = []() -> s32 {
        std::cout << "Cleaning up Module\n";
        return 0;
    };

}; // struct Protocol

} // namespace ykz

#endif /* PROTOCOL_HPP_ */
