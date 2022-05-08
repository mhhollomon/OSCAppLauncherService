#include <libconfig.h++>
#include <iostream>
#include <memory>

using config_ptr = std::unique_ptr<libconfig::Config> ;

config_ptr read_config_file(std::string filename, bool echo = false);