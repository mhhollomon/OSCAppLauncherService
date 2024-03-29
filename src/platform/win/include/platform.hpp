//
// Windows platform
//

#include <platform_base.hpp>
#include <osc_socket.hpp>


namespace Platform {


	// Must include the directory separator on the end
	std::string get_cfg_directory();


	// Process Control

	void launch_app(std::string path_or_name);
	void launch_app(std::string path_or_name, std::string arg);

	// Networking

	std::unique_ptr<OSCSocket> create_socket(const std::string iface, int port, SocketDirection direction);


} // end namespace Platform



