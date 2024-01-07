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

using namespace std::literals::string_literals;

const std::string  CONFIG_SCHEMA = R"DELIM(
server! : {
	interface : { _t : string, _d : "127.0.0.1"; },
	port : { _t : int, _d : 8888 };
}

launch! : {
	*! : string
}
)DELIM";


constexpr int DEFAULT_PORT = 8888;	//The port on which to listen for incoming data

std::map<std::string, std::string> program_map;

int main(int argc, char** argv) {

	auto cfg_file = Pl::get_cfg_directory() + "OALServer.cfg";

	if (argc >= 2) {
		cfg_file = std::string(argv[1]);
	}

	std::cout << "using config file = " << cfg_file << "\n";

	sc_ptr cfg = read_config_file(cfg_file, CONFIG_SCHEMA);

	
	if (cfg->has_errors()) {
		cfg->stream_errors(std::cerr);
		return(EXIT_FAILURE);
	}

	int port = cfg->at_path("server.port").get<int>();
	std::string host_addr = cfg->at_path("server.interface").get<std::string>();

	std::cout << "using host address : " << host_addr << "\n";
	std::cout << "using port : " << port << "\n";

	auto& launch_settings = cfg->at_path("launch");

	for (const auto &[name, setting] : launch_settings.enumerate()) {
		program_map.insert(std::make_pair(name, setting.get<std::string>().c_str()));
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
