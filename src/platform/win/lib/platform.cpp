
#include <platform.hpp>

#pragma comment(lib,"ws2_32.lib") //Winsock Library


std::string Platform::get_cfg_file_name() {
	std::string profile_path = getenv("USERPROFILE");
	std::string ini_file = profile_path + "\\Documents\\" + CFG_FILE_NAME;

	return ini_file;
}