/*
 * Created by Costa Bushnaq
 *
 * 04-10-2021 @ 22:02:43
 *
 * see LICENSE
*/

#ifndef _HOST_HPP
#define _HOST_HPP

#include <memory>

#include "og/Poll.hpp"

namespace ykz {

class Host {
	std::unique_ptr<og::Poll> m_poll;


}; // class Host

} // namespace ykz

#endif /* _HOST_HPP */
