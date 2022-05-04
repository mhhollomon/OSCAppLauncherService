
#include <platform.hpp>

#pragma comment(lib,"ws2_32.lib") //Winsock Library


std::string Platform::get_ini_file_name() {
	std::string profile_path = getenv("HOME");
	std::string ini_file = profile_path + "/.oal/" + INI_FILE_NAME;

	return ini_file;
}