/*
 * Created by Costa Bushnaq
 *
 * 05-05-2021 @ 20:11:15
 *
 * see LICENSE
*/

#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <mutex>
#include <string>

#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace ykz {

class Logger {
	static std::mutex  m_mut;
	static std::FILE*  m_file;
	static std::string m_prefix;
	thread_local static int m_code;

public:
	static Logger g_logger;

	static Logger& set_prefix(std::string const&);
	static Logger& reset_prefix();
	static Logger& set_code(int); //! code is thread-local

	static Logger& log(std::string const, ...);

protected:
	Logger();
	Logger(Logger const&) = delete;
	void operator=(Logger const&) = delete;

	static void log(std::string const&, va_list);

}; // class Logger

} // namespace ykz

#endif /* _LOGGER_HPP */
