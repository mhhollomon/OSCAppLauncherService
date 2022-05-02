// OALSend.cpp : Defines the entry point for the application.
//

/*
	Simple udp client
*/
#include <iostream>

#include <inipp.h>

#include <WinOSCSocket.h>

int main(int argc, char**argv) {
	if (argc < 2)
		throw std::runtime_error("Not enough arguments");

	int port = 0;
	inipp::extract(std::string(argv[1]), port);
	std::cout << "using port : " << port << "\n";

	auto osc_msg = OSCMessage(argv[2]);
		
	std::unique_ptr<OSCSocket> socket = WinOSCSocketFactory().create_socket(port, OSCSocket::Direction::WRITE);




	//send the message
	socket->send_osc(osc_msg);


	return 0;

}