
#include <platform.hpp>


std::string Platform::get_cfg_file_name() {
	std::string profile_path = getenv("HOME");
	std::string ini_file = profile_path + "/.oal/" + CFG_FILE_NAME;

	return ini_file;
}
