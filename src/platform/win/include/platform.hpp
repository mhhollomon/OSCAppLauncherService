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

		CreateProcessA(
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

