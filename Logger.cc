/*
 * Created by Costa Bushnaq
 *
 * 05-05-2021 @ 20:12:05
*/

#include "Logger.hpp"

using namespace ykz;

Logger           Logger::g_logger;
std::mutex       Logger::m_mut;
std::FILE*       Logger::m_file{stderr};
std::string      Logger::m_prefix{"server: "};
thread_local int Logger::m_code{0};

Logger::Logger()
{

}

Logger& Logger::set_prefix(std::string const& prefix)
{
	m_prefix = prefix;

	return g_logger;
}

Logger& Logger::reset_prefix()
{
	m_prefix = "log: ";

	return g_logger;
}

Logger& Logger::set_code(int code)
{
	m_code = code;

	return g_logger;
}

Logger& Logger::log(std::string const fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log(fmt, ap);
	va_end(ap);

	return g_logger;
}

void Logger::log(std::string const &fmt, va_list ap)
{
	std::lock_guard<std::mutex> lck{m_mut};
	{
		auto f = fmt.c_str();

		if (!m_prefix.empty())
			fprintf(m_file, "%s", m_prefix.c_str());

		vfprintf(m_file, f, ap);

		if (f[0] && f[fmt.size() - 1] == ':')
		{
			if (m_code == 0)
				fprintf(m_file, " %s\n", strerror(errno));
			else
				fprintf(m_file, " %s\n", strerror(m_code));
		}
		else
		{
			fputc('\n', m_file);
		}
		m_code = 0;
	}
}
