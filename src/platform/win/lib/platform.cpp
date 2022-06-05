#include <platform.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#pragma comment(lib,"ws2_32.lib") //Winsock Library

namespace Platform {

//=============================================================
std::string get_cfg_directory() {
	std::string profile_path = getenv("USERPROFILE");
	std::string dir = profile_path + "\\Documents\\";

	return dir;
}

//=============================================================


void launch_app(std::string path_or_name) {
	launch_app(path_or_name, "");
}

void launch_app(std::string path_or_name, std::string arg) {
	char* buffer = 0;

	if (!arg.empty()) {

		std::cout << "There is an arg: '" << arg << "'\n";

		// path to executable needs to be in quotes. Probably ought to quote everything.
		std::string true_arg = std::string("\"") + path_or_name + std::string("\"") + std::string(" ") + arg;

		// the args arg cannot be const, so copy to local buffer - sigh
		buffer = new char[true_arg.size() + 1];

		memset(buffer, 0, true_arg.size() + 1);

		//std::cout << "memccpy\n";

		memcpy(buffer, true_arg.c_str(), true_arg.size());

	}

	//std::cout << "after memccpy\n";

	std::cout << "exe = " << path_or_name << "\n";
	if (buffer)
		std::cout << "arg = '" << buffer << "' size = " << strlen(buffer) << "\n";


	auto ret_code = ShellExecuteA(NULL, NULL, path_or_name.c_str(), buffer, NULL, SW_SHOWNORMAL);
	//std::cout << "After call to CP\n";

	if (buffer)
		delete[] buffer;

	//std::cout << "After call to delete.\n";

	if (!ret_code) {
		//std::cout << "About to throw\n";
		throw std::runtime_error("Could not start application");
	}


}

// ===============================================================

std::unique_ptr<OSCSocket> create_socket(int port, SocketDirection direction) {
	return std::make_unique<OSCSocket>(port, direction);
}



} // end namespace Platform


