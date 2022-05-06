
#include <platform.hpp>

#include <unistd.h>
#include <exception>


namespace Platform {

// ===========================================================================
std::string get_cfg_file_name() {
	std::string profile_path = getenv("HOME");
	std::string ini_file = profile_path + "/.oal/" + CFG_FILE_NAME;

	return ini_file;
}

// ===========================================================================
void launch_app(std::string path_or_name) {
    launch_app(path_or_name, "");
}

void launch_app(std::string path_or_name, std::string arg) {
    auto pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Fork did not work");
    } else if(pid > 0) {

        // in the parent;
        //
    } else {
        // in the child;
        const char *arg_ptr = 0;
        if (!arg.empty()) {
            arg_ptr = arg.c_str();
        }
        execl(path_or_name.c_str(), path_or_name.c_str(), arg_ptr, (const char *)0);
        exit(0);
    }
}
// ===========================================================================
std::unique_ptr<OSCSocket> create_socket(int port, SocketDirection direction) {
    return std::make_unique<OSCSocket>(port, direction);
}

} // end namespace Platform
