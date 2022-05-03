//
// Windows platform
//

#include <platform_base.hpp>
#include <osc_socket.hpp>

class Platform : public PlatformBase<OSCSocket> {
public:
	virtual std::string get_ini_file_name();
	//virtual std::unique_ptr<OSCSocket> create_socket(int port, SocketDirection direction);

};