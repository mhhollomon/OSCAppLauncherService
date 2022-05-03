// base platform class

#pragma once

#include <socket_base.hpp>

#include <string>
#include <memory>


template <class Socket_T> class PlatformBase {

public:
	// returns full path and name of the ini file.
	virtual std::string get_ini_file_name() = 0;

	virtual std::unique_ptr<Socket_T> create_socket(int port, SocketDirection direction) {
		return std::make_unique<Socket_T>(port, direction);
	}

protected :
	const std::string INI_FILE_NAME = "OALServer.ini";
};