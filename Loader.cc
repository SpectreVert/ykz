/*
 * Created by Costa Bushnaq
 *
 * 03-05-2021 @ 22:05:20
*/

#include "Loader.hpp"
#include "Logger.hpp"

#include <exception>
#include <filesystem>

using namespace ykz;

std::size_t Loader::load_from_path(std::string const& dpath)
{
	std::error_code code;
	std::size_t i{0};

	for (const auto& entry : std::filesystem::directory_iterator(dpath, code))
	{
		if (code.value())
		{
			Logger::set_code(code.value()).log("directory_iterator:");
			continue;
		}
		
		auto fname = entry.path().string();

		// check if the suffix at least fits in the file name
		if (fname.size() < k_suffix.size())
			continue;

		// check if the file name ends by the suffix
		if (!std::equal(k_suffix.rbegin(), k_suffix.rend(), fname.rbegin()))
			continue;

		// now try to create the ModuleInstance from this shared library
		auto instance = ModuleInstance::load_instance(fname);

		// just log success -- error is logged in load_instance()
		if (instance)
		{
			Logger::log("loader: loaded %s", fname.c_str());
			m_modules[fname] = std::move(instance);
			++i;
		}
	}

	return i;
}

std::unique_ptr<Loader::ModuleInstance>
Loader::ModuleInstance::load_instance(std::string const& fname)
{
	auto binary = dlopen(fname.c_str(), RTLD_NOW);

	if (!binary)
	{
		Logger::log("loader: %s", dlerror());
		return std::unique_ptr<ModuleInstance>();
	}

	auto instance = std::make_unique<ModuleInstance>();
	instance->binary = std::shared_ptr<void>(binary, dtor);

	auto ctor = dlsym(
		std::reinterpret_pointer_cast<void*>(instance->binary).get(),
		"constructor"
	);

	if (!ctor)
	{
		Logger::log("loader: %s", dlerror());
		return std::unique_ptr<ModuleInstance>();
	}

	instance->ctor = reinterpret_cast<ConstructorType*>(ctor);
	instance->module = std::shared_ptr<IModule>(instance->ctor());

	return instance;
}

void Loader::ModuleInstance::dtor(void* handler)
{
	// this thing is gonna execute once anyway so
	// we're not gonna be bothering checking things
	// and priting errors
	dlclose(handler);
}
