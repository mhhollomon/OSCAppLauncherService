// base platform class

#pragma once

#include <string>
class PlatformBase {

public:
	// returns full path and name of the ini file.
	virtual std::string get_ini_file_name() = 0;

protected :
	const std::string INI_FILE_NAME = "OALServer.ini";
};