/*
 * Created by Costa Bushnaq
 *
 * 03-05-2021 @ 22:05:28
 *
 * see LICENSE
*/

#ifndef _LOADER_HPP
#define _LOADER_HPP

#include "generic/IModule.hpp"

#include <map>
#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <dlfcn.h>

namespace ykz {

class Loader {
	struct ModuleInstance {
		using ConstructorType = IModule*();

		virtual ~ModuleInstance() = default;

		std::unique_ptr<ModuleInstance> static load_instance(std::string const&);
		void static dtor(void*);

		std::shared_ptr<void> binary;
		std::shared_ptr<IModule> module;
		std::function<ConstructorType> ctor;
	};

	std::map<std::string, std::unique_ptr<ModuleInstance>> m_modules;

public:
	std::string_view constexpr static k_suffix = ".so";

	virtual ~Loader() = default;
	Loader() = default;

	std::size_t load_from_path(std::string const&);
	std::size_t unload_all();
	bool unload(std::string const&);

	std::vector<std::string> loaded_modules() const;
	std::weak_ptr<IModule> instance(std::string const&) const;

}; // class Loader

} // namespace ykz

#endif /* _LOADER_HPP */
