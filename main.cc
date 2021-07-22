/*
 * Created by Costa Bushnaq
 *
 * 03-05-2021 @ 20:44:56
*/

#include "Loader.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include "Connection.hpp"

#include <chrono>
#include <thread>

#include <fcntl.h>
#include <sys/stat.h>

using namespace ykz;

int main(int ac, char* av[])
{
	Loader loader;

	auto s = loader.load_from_path("./");
}
