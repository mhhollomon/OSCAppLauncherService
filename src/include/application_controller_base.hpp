#pragma once

class ApplicationControllerBase {
public:
	virtual void launch_app(std::string path_or_name) = 0;
	virtual ~ApplicationControllerBase() = default;
};