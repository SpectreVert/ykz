/*
 * Created by Costa Bushnaq
 *
 * 05-05-2021 @ 20:11:15
 *
 * see LICENSE
*/

#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include "thread/Concurrent.hpp"

#include <iostream>
#include <string>

#include <cstdarg>

namespace ykz {

/*
class Logger {
public:
	struct 

	static void set_prefix(std::string const&);
	static void reset_prefix();

	static void log(std::string const&);
	static void log(std::string const&, ...);
	static void log(std::string const&, va_list);

private:
	static Logger m_instance;
	static Concurrent<std::ostream> m_stream;
	static std::string m_prefix;

	Logger();
	Logger(Logger const&) = delete;

	void operator=(Logger const&) = delete;

}; // class Logger */

} // namespace ykz

#endif /* _LOGGER_HPP */
