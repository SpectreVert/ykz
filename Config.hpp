/*
 * Created by Costa Bushnaq
 *
 * 17-05-2021 @ 23:54:38
 *
 * see LICENSE
*/

#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include <string>

namespace ykz {

/* configuration which is used to customize the server's behaviour */
struct Config {
	struct Core {
#if 0
		/// Right now we have one poller and this parameter is unused
		std::size_t nb_poller  = 2;  /* number of poller threads */
#endif
		std::size_t nb_slots   = 64; /* number of clients per poller */
		std::size_t nb_workers = 4;  /* number of worker threads */

		bool msg_order = true; /* send responses in same order as requests */
	} core;

	struct Logger {
		std::string output_file = "yzk.log";
	} logger;

	struct Loader {
		std::string modules_directory = "mod";
	} loader;

}; // struct Config

} // namespace ykz

#endif /* _CONFIG_HPP */
