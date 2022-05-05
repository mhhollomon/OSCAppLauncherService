//
// Windows platform
//

#include <platform_base.hpp>
#include <osc_socket.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class ApplicationController : public ApplicationControllerBase {
public:
	virtual void launch_app(std::string path_or_name) {
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		auto ret_code = CreateProcessA(
			path_or_name.c_str(),
			0,
			0,
			0,
			false,
			CREATE_NEW_CONSOLE,
			0,
			0,
			&si,
			&pi
		);

		if (!ret_code) {
			throw std::runtime_error("Could not start application");
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	virtual void launch_app(std::string path_or_name, std::string arg) {
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		// path to executable needs to be in quotes. Probably ought to quote everything.
		std::string true_arg = std::string("\"") + path_or_name + std::string("\"") + std::string(" ") + arg;

		// the args arg cannot be const, so copy to local buffer - sigh
		char* buffer = new char[true_arg.size()+1];

		memset(buffer, 0, true_arg.size() + 1);

		//std::cout << "memccpy\n";

		memcpy(buffer, true_arg.c_str(), true_arg.size());

		//std::cout << "after memccpy\n";

		std::cout << "exe = " << path_or_name << "\n";
		std::cout << "arg = '" << buffer << "' size = " << strlen(buffer) << "\n";

		auto ret_code = CreateProcessA(
			path_or_name.c_str(),
			buffer,
			0,
			0,
			false,
			CREATE_NEW_CONSOLE,
			0,
			0,
			&si,
			&pi
		);

		//std::cout << "After call to CP\n";

		delete [] buffer;

		//std::cout << "After call to delete.\n";

		if (!ret_code) {
			//std::cout << "About to throw\n";
			throw std::runtime_error("Could not start application");
		}


		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

};

class Platform : public PlatformBase<OSCSocket, ApplicationController> {
public:
	virtual std::string get_cfg_file_name();
	//virtual std::unique_ptr<OSCSocket> create_socket(int port, SocketDirection direction);
	//ApplicationController create_application_controller();

};

