#pragma once

#include <socket_base.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>

class OSCSocket : public OSCSocketBase {
	static constexpr int BUFLEN = 512;
	static constexpr char INTERFACE[] = "127.0.0.1";


	int socket_;
	struct sockaddr_in socket_address_;

public:

	OSCSocket(int port, SocketDirection direction) {

		//Create a socket
		if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			throw std::runtime_error("Could not create socket");
		}

		std::cout << "Socket created.\n";

		//Prepare the sockaddr_in structure
		socket_address_.sin_family = AF_INET;
		inet_pton(socket_address_.sin_family, INTERFACE, &socket_address_.sin_addr.s_addr);
		socket_address_.sin_port = htons(port);

		if (direction == SocketDirection::READ) {

			//Bind
			if (bind(socket_, (struct sockaddr*)&socket_address_, sizeof(socket_address_)) < 0) {
				throw std::runtime_error("Could not bind");
			}
			std::cout << "Bind done\n";
		}

	}

	virtual std::string receive_raw() {
		int recv_len;
		// Add one to make sure we have a NUL on the end when reading BUFLEN bytes.
		char buf[BUFLEN+1] = { 0 };

		//try to receive some data, this is a blocking call
		if ((recv_len = recv(socket_, buf, BUFLEN, 0)) < 0) {
			throw std::runtime_error("recv failed");
		}

		return std::string{ buf, (size_t)recv_len };

	}

	virtual void send_raw(std::string msg) {
		std::cout << "sending " << msg.size() << " bytes\n";
		if (sendto(socket_, msg.c_str(), (int)msg.size(), 0, (struct sockaddr*)&socket_address_, sizeof(socket_address_)) < 0) {
			throw std::runtime_error("sendto() failed");
		}
	}

};
