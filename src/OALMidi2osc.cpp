// OALMidi2osc.cpp 
//

#include <iostream>
#include <sstream>

#include <platform.hpp>
#include <config.hpp>

namespace Pl = Platform;

int main(int argc, char** argv) {

	std::string cfg_file = Pl::get_cfg_directory() + "OALMidi2osc.cfg";

	if (argc >= 2) {
		cfg_file = std::string(argv[1]);
	}

	std::cout << "using config file = " << cfg_file << "\n";


	config_ptr cfg;

	try {
		cfg = read_config_file(cfg_file, true);
	}
	catch (const libconfig::FileIOException& fioex) {
		std::cerr << "I/O error while reading file: " << fioex.what() << std::endl;
		return(EXIT_FAILURE);
	}
	catch (const libconfig::ParseException& pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			<< " - " << pex.getError() << std::endl;
		return(EXIT_FAILURE);
	}


	return(EXIT_SUCCESS);
}
