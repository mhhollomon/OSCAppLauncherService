// OALServer.cpp 
//

#include <config.hpp>
#include <platform.hpp>
namespace Pl = Platform;

#include <exception>
#include <map>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>


constexpr int DEFAULT_PORT = 8888;	//The port on which to listen for incoming data

std::map<std::string, std::string> program_map;

int main(int argc, char** argv) {

	auto cfg_file = Pl::get_cfg_directory() + "OALServer.cfg";

	if (argc >= 2) {
		cfg_file = std::string(argv[1]);
	}

	std::cout << "using config file = " << cfg_file << "\n";


	config_ptr cfg;

	try {
		cfg = read_config_file(cfg_file, true);
	}
	catch (const libconfig::FileIOException& fioex) {
		std::cerr << "I/O error while reading file." << std::endl;
		return(EXIT_FAILURE);
	}
	catch (const libconfig::ParseException& pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			<< " - " << pex.getError() << std::endl;
		return(EXIT_FAILURE);
	}


	int port = DEFAULT_PORT;
	std::string host_addr = "127.0.0.1";

	cfg->lookupValue("server.interface", host_addr);
	std::cout << "using host address : " << host_addr << "\n";

	cfg->lookupValue("server.port", port);
	std::cout << "using port : " << port << "\n";

	if (cfg->exists("launch")) {
		auto& launch_settings = cfg->lookup("launch");
		if (!launch_settings.isGroup()) {
			std::cerr << "The 'launch' configuration must be a group\n";
			return(EXIT_FAILURE);
		}

		for (int index = 0; index < launch_settings.getLength(); ++index) {
			program_map.insert(std::make_pair(std::string(launch_settings[index].getName()), std::string(launch_settings[index].c_str())));
		}
	}


	std::unique_ptr<OSCSocket> socket = Pl::create_socket(host_addr, port, SocketDirection::READ);

	while (1) {
		std::cout << "Waiting for data...\n";
		fflush(stdout);

		auto msg = socket->recieve_osc();

		std::cout << "Data: ";
		msg.to_stream(std::cout);

		// Processing.
		if (msg.address().compare("/exit") == 0) {
			std::cout << "Exiting\n";
			break;
		}

		if (msg.hierarchy_match("/launch")) {
			auto key = msg.method_name();
			if (program_map.find(key) != program_map.end()) {


				if (msg.arg_count() > 0 && msg.get_arg(0).isString()) {
					const std::string arg = msg.get_arg(0).get<std::string>();
					std::cout << "Starting program " << program_map.at(key) << "( " << arg << " )\n";

					Pl::launch_app(program_map.at(key), arg);

				}
				else {
					std::cout << "Starting program " << program_map.at(key) << "\n";

					Pl::launch_app(program_map.at(key));

				}
			}
			else {
				std::cout << "Unknown key " << key << " - ignoring command\n";
			}
		}
		else {
			std::cout << "Unknown heirarchy - ignoring\n";
		}
	}
}
