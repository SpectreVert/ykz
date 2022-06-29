/*
 * Created by Costa Bushnaq
 *
 * 03-05-2021 @ 20:44:56
*/

#include "Loader.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include "Connection.hpp"

#include "TaskBucket.hpp"

#include <chrono>
#include <thread>

#include <fcntl.h>
#include <sys/stat.h>

#include <iostream>

using namespace ykz;

int num()
{
	return 20;
}

int main(int ac, char* av[])
{
	TaskBucket task_bucket{4};

	std::vector<std::future<int>> v;

	for (std::size_t i = 0; i < 100; ++i) {	
		v.push_back(task_bucket.schedule_task<int>(std::bind(num)));
	}

	for (std::size_t i = 0; i < 100; ++i) {	
		std::cout << v[i].get() << '\n';
	}

}
