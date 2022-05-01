// OALSend.cpp : Defines the entry point for the application.
//

/*
	Simple udp client
*/
#include <iostream>

#include <WinOSCSocket.h>

constexpr int PORT = 8888;	//The port on which to listen for incoming data

int main(int argc, char**argv) {
	if (argc < 1)
		throw std::runtime_error("Not enough arguments");

	auto osc_msg = OSCMessage(argv[1]);
		
	std::unique_ptr<OSCSocket> socket = WinOSCSocketFactory().create_socket(PORT, OSCSocket::Direction::WRITE);




	//send the message
	socket->send_osc(osc_msg);


	return 0;

}