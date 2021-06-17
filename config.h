/* Server core configuration */
std::size_t Config::Core::nb_threads{4};
std::size_t Config::Core::nb_slots{64};

/* Logger configuration */
std::string Config::Logger::output_file{""};

/* Modules loader configuration */
std::string Config::Loader::modules_directory{"modules"};
