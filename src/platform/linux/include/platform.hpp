//
// Windows platform
//

#include <platform_base.hpp>
#include <osc_socket.hpp>


#include <unistd.h>
#include <exception>

class ApplicationController : public ApplicationControllerBase {
public:
	virtual void launch_app(std::string path_or_name) {
        auto pid = fork();
        if (pid == -1) {
            throw std::runtime_error("Fork did not work");
        } else if(pid > 0) {

            // in the parent;
            //
        } else {
            // in the child;
            execl(path_or_name.c_str(), path_or_name.c_str());
            exit(0);
        }
	}
};

class Platform : public PlatformBase<OSCSocket, ApplicationController> {
public:
	virtual std::string get_ini_file_name();
	//virtual std::unique_ptr<OSCSocket> create_socket(int port, SocketDirection direction);
	//ApplicationController create_application_controller();

};

