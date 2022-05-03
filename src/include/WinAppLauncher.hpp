#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class WinAppLauncher {
public:
	void launch_app(std::string path_or_name) {
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