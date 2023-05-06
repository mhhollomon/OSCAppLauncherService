// OALSend.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <sstream>

#include <platform.hpp>

namespace Pl = Platform;

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
	std::cout << "argc = " << argc << "\n";

	if (argc < 3) {
		std::cerr << "Not enough arguments\n";
		exit(EXIT_FAILURE);
	}

	int port = 0;
	extract(std::string(argv[1]), port);
	std::cout << "using port : " << port << "\n";

	auto osc_msg = OSCMessage(argv[2]);

	if (argc > 3) {
		auto arg_spec = std::string_view(argv[3]);
		if (arg_spec[1] != ':') {
			std::cerr << "Argument not in the correct format\n";
			exit(EXIT_FAILURE);
		}
		osc_msg.add_arg(arg_spec[0], arg_spec.substr(2));
	}

	std::cout << "Got here\n";
	osc_msg.to_stream(std::cout);
		
	std::unique_ptr<OSCSocket> socket = Pl::create_socket("127.0.0.1", port, SocketDirection::WRITE);


	//send the message
	socket->send_osc(osc_msg);


	return 0;

}