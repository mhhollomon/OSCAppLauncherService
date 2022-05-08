//
// Windows platform
//

#include <platform_base.hpp>
#include <osc_socket.hpp>


namespace Platform {

	std::string get_cfg_directory();


	// Process Control

	void launch_app(std::string path_or_name);
	void launch_app(std::string path_or_name, std::string arg);

	// Networking

	std::unique_ptr<OSCSocket> create_socket(int port, SocketDirection direction);


} // end namespace Platform



