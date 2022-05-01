// OALServer.cpp 
//

#include <map>

#include <iostream>

#include <WinOSCSocket.h>
#include <WinAppLauncher.hpp>

constexpr int PORT = 8888;	//The port on which to listen for incoming data

const std::map<std::string, std::string> program_map = {
	{ "notepad" , "C:\\Program Files\\Notepad++\\notepad++.exe"},
};

auto socket_factory = WinOSCSocketFactory();
auto app_launcher = WinAppLauncher();

int main() {

	std::unique_ptr<OSCSocket> socket = socket_factory.create_socket(PORT, OSCSocket::Direction::READ);

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
