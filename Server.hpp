/*
 * Created by Costa Bushnaq
 *
 * 24-09-2021 @ 12:20:16
 *
 * see LICENSE
*/

#ifndef _SERVER_HPP
#define _SERVER_HPP

#include <queue>
#include <unordered_map>

#include "Config.hpp"
#include "Connection.hpp"
#include "TaskBucket.hpp"

namespace ykz {

class Server {
#if 0
	std::shared_ptr<og::net::TcpListener> m_listener;
	std::unique_ptr<og::net::Poll> m_poller;
#endif

	std::shared_ptr<TaskBucket> m_bucket;

	std::unordered_map<std::size_t, Connection> m_clients;
	std::queue<std::size_t> m_free_ids;

public:
	virtual ~Server() = default;
	Server(Config const& config);

	/* void init(std::shared_ptr<og::net::TcpListener> listener); */

}; // class Server

} // namespace ykz

#endif /* _SERVER_HPP */
