#include <config.hpp>

#include <exception>


config_ptr read_config_file(std::string filename, bool echo) {

	auto cfg = std::make_unique<libconfig::Config>();

	cfg->readFile(filename);

	if (echo) {

		// Make sure everybody agrees on where we are in the output stream.
		std::cout.flush();
		std::cout.sync_with_stdio();

		// libconfig doesn't give a stream interface - grr 
		cfg->write(stdout);
	}

	return cfg;
}
