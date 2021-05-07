/*
 * Created by Costa Bushnaq
 *
 * 03-05-2021 @ 20:44:56
*/

#include "Logger.hpp"

using namespace ykz;

int main(int ac, char* av[])
{
	Concurrent<std::string> l("zonz");
	std::string zonzibar("azeaze");

	auto fut = l([=](auto s){
		std::cerr << zonzibar << std::endl;
	});

	fut.wait();
}
