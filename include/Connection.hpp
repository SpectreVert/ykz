/*
 * Created by Costa Bushnaq
 *
 * 02-07-2021 @ 00:11:13
 *
 * see LICENSE
*/

#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

#include <deque>
#include <future>
#include <vector>

#include "Message.hpp"

namespace ykz {

class Connection {
	int m_stamp{0};
	/* when getting notified that some future has completed for this
	connection; either check only the first element (if msg_order is set)
	or check any of them for completion. If the checked response is indeed
	ready, remove it, send it to the client and redo. */
	std::deque<std::future<Message>> m_pending_reponses;
	/* og::net::TcpStream m_stream; */

public:
	/* og::net::TcpStream& stream(); */
	int new_stamp();

	std::vector<Message> try_get_completed(bool msg_order);

}; // struct Connnection

} // namespace ykz

#endif /* _CONNECTION_HPP */
