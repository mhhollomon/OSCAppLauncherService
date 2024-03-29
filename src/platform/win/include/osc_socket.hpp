#pragma once

#include <socket_base.hpp>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdexcept>
#include <string>

class OSCSocket : public OSCSocketBase {
	static constexpr int WS_VER = 0x0202;
	static constexpr int BUFLEN = 512;


	SOCKET socket_;
	struct sockaddr_in socket_address_;

public:

	OSCSocket(const std::string iface, int port, SocketDirection direction) {
		WSADATA wsa;

		//Initialise winsock
		std::cout << "\nInitialising Winsock...";
		if (WSAStartup(OSCSocket::WS_VER, &wsa) != 0) {
			throw std::runtime_error("Winsock Init Failed");
			//printf("Failed. Error Code : %d", WSAGetLastError());
		}
		std::cout << "Initialised.\n";

		//Create a socket
		if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
			//std::cout << "Could not create socket : " << WSAGetLastError() << "\n";
			throw std::runtime_error("Could not create socket");
		}

		std::cout << "Socket created.\n";

		//Prepare the sockaddr_in structure
		socket_address_.sin_family = AF_INET;
		inet_pton(socket_address_.sin_family, iface.c_str(), &socket_address_.sin_addr.S_un.S_addr);
		socket_address_.sin_port = htons(port);

		if (direction == SocketDirection::READ) {

			//Bind

			std::cout << "Binding to " << iface << ":" << port << "\n";
			if (bind(socket_, (struct sockaddr*)&socket_address_, sizeof(socket_address_)) == SOCKET_ERROR) {
				//std::cout << "Bind failed with error code : " << WSAGetLastError();
				//exit(EXIT_FAILURE);
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
		if ((recv_len = recv(socket_, buf, BUFLEN, 0)) == SOCKET_ERROR) {
			throw std::runtime_error("recv failed");
			// std::cout << "recv() failed with error code : " << WSAGetLastError();
		}

		return std::string{ buf, (size_t)recv_len };

	}

	virtual void send_raw(std::string msg) {
		std::cout << "sending " << msg.size() << " bytes\n";
		if (sendto(socket_, msg.c_str(), (int)msg.size(), 0, (struct sockaddr*)&socket_address_, sizeof(socket_address_)) == SOCKET_ERROR) {
			throw std::runtime_error("sendto() failed");
			//printf("sendto() failed with error code : %d", WSAGetLastError());
		}
	}

	virtual void close() {
		closesocket(socket_);
	}

	virtual ~OSCSocket() {
		close();
		// Makes an ugly assumption.
		WSACleanup();

	}

};