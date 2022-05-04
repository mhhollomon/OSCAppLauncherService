// base platform class

#pragma once

#include <socket_base.hpp>
#include <application_controller_base.hpp>

#include <string>
#include <memory>


template <class Socket_T, class AppCon_T> class PlatformBase {

public:
	// returns full path and name of the config file.
	virtual std::string get_cfg_file_name() = 0;

	virtual std::unique_ptr<Socket_T> create_socket(int port, SocketDirection direction) {
		return std::make_unique<Socket_T>(port, direction);
	}

	 AppCon_T create_application_controller() {
		return AppCon_T{};
	}

protected :
	const std::string CFG_FILE_NAME = "OALServer.cfg";
};