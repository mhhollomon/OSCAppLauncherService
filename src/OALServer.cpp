// OALServer.cpp 
//

#include "inipp.h"

#include <map>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <WinOSCSocket.h>
#include <WinAppLauncher.hpp>
#include <platform.hpp>

const std::string rel_ini_path = "\\Documents\\OALServer.ini";

constexpr int DEFAULT_PORT = 8888;	//The port on which to listen for incoming data

std::map<std::string, std::string> program_map;

auto socket_factory = WinOSCSocketFactory();
auto app_launcher = WinAppLauncher();

Platform platform;

int main() {
	inipp::Ini<char> ini;

	auto ini_file = platform.get_ini_file_name();


	std::cout << "using ini file = " << ini_file << "\n";

	std::ifstream is(ini_file);
	ini.parse(is);
	ini.generate(std::cout);

	int port = DEFAULT_PORT;
	inipp::extract(ini.sections["Server"]["port"], port);

	if (ini.sections.find("Applications") != ini.sections.end()) {
		program_map = ini.sections["Applications"];
	}
	else {
		throw std::runtime_error("No applications found in the ini file.");
	}

	std::cout << "using port : " << port << "\n";

	std::unique_ptr<OSCSocket> socket = socket_factory.create_socket(port, OSCSocket::Direction::READ);

	while (1) {
		std::cout << "Waiting for data...\n";
		fflush(stdout);

		auto msg = socket->recieve_osc();

		std::cout << "Data: " << msg.address() << "\n";
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
