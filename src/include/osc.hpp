#pragma once

#include <string>
#include <string_view>

class OSCMessage {
	std::string addr_;
public:
	std::string method_name();
	std::string address() { return addr_; };

	bool hierarchy_match(std::string path) {
		return (addr_.rfind(path + "/", 0) == 0);
	}

	std::string to_data();

	OSCMessage(std::string addr) { 
		addr_ = addr;
	}

	static OSCMessage from_data(std::string_view data);

private :
	OSCMessage() = default;

};
