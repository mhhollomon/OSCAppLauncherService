#include <config.hpp>

#include <exception>
#include <string>

sc_ptr read_config_file(std::string &filename, std::string const &schema, 
        bool echo)
{
	auto cfg = std::make_unique<simpleConfig::Config>();

	if (cfg->set_schema(schema)) {
		cfg->parse_file(filename);
	}

	return cfg;
}


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
