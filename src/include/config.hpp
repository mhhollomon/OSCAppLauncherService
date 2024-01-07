#include <libconfig.h++>
#include <iostream>
#include <memory>
#include <simpleConfig.hpp>

using config_ptr = std::unique_ptr<libconfig::Config> ;

config_ptr read_config_file(std::string filename, bool echo = false);

using sc_ptr = std::unique_ptr<simpleConfig::Config>;

sc_ptr read_config_file(std::string &filename, std::string const &schema, 
        bool echo = false);
