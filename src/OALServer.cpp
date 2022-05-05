// OALServer.cpp 
//

#include <libconfig.h++>
#include <platform.hpp>

#include <exception>
#include <map>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>


constexpr int DEFAULT_PORT = 8888;	//The port on which to listen for incoming data

std::map<std::string, std::string> program_map;

int main() {
	Platform platform;

	libconfig::Config cfg;


	auto cfg_file = platform.get_cfg_file_name();
	auto app_launcher = platform.create_application_controller();


	std::cout << "using config file = " << cfg_file << "\n";

	try {
		cfg.readFile(cfg_file);

	} catch (const libconfig::FileIOException& fioex)	{
		std::cerr << "I/O error while reading file." << std::endl;
		return(EXIT_FAILURE);
	}	catch (const libconfig::ParseException& pex)  {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			<< " - " << pex.getError() << std::endl;
		return(EXIT_FAILURE);
	}

	// Make sure everybody agrees on where we are in the output stream.
	std::cout.flush();
	std::cout.sync_with_stdio();

	// libconfig doesn't give a stream interface - grr 
	cfg.write(stdout);

	int port = DEFAULT_PORT;
	cfg.lookupValue("server.port", port);
	std::cout << "using port : " << port << "\n";

	if (cfg.exists("launch")) {
		auto& launch_settings = cfg.lookup("launch");
		if (!launch_settings.isGroup()) {
			std::cerr << "The 'launch' configuration must be a group\n";
			return(4);
		}

		for (int index = 0; index < launch_settings.getLength(); ++index) {
			program_map.insert(std::make_pair(std::string(launch_settings[index].getName()), std::string(launch_settings[index].c_str())));
		}
	}


	std::unique_ptr<OSCSocket> socket = platform.create_socket(port, SocketDirection::READ);

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

				std::cout << "Starting program " << program_map.at(key) << "\n";

				app_launcher.launch_app(program_map.at(key));
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
