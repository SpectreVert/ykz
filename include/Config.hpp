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

//! Configuration which is used to customize
//! variables throughout the program.
//!
struct Config {

	static struct Core {
		static std::size_t nb_threads;
		static std::size_t nb_slots;
	} core;

	struct Logger {
		static std::string output_file;
	} logger;

	struct Loader {
		static std::string modules_directory;
	} loader;

}; // struct Config

} // namespace ykz

#endif /* _CONFIG_HPP */
