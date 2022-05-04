// OALSend.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <sstream>

#include <platform.hpp>

// Helpers
template <typename CharT, typename T>
inline bool extract(const std::basic_string<CharT>& value, T& dst) {
	CharT c;
	std::basic_istringstream<CharT> is{ value };
	T result;
	if ((is >> std::boolalpha >> result) && !(is >> c)) {
		dst = result;
		return true;
	}
	else {
		return false;
	}
}

template <typename CharT>
inline bool extract(const std::basic_string<CharT>& value, std::basic_string<CharT>& dst) {
	dst = value;
	return true;
}



int main(int argc, char**argv) {
	Platform platform;

	if (argc < 2)
		throw std::runtime_error("Not enough arguments");

	int port = 0;
	extract(std::string(argv[1]), port);
	std::cout << "using port : " << port << "\n";

	auto osc_msg = OSCMessage(argv[2]);
		
	std::unique_ptr<OSCSocket> socket = platform.create_socket(port, SocketDirection::WRITE);


	//send the message
	socket->send_osc(osc_msg);


	return 0;

}