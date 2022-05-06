#pragma once

#include <osc.hpp>

#include <memory>
#include <iostream>

enum class SocketDirection {
	READ,
	WRITE,
};

class OSCSocketBase {
public :

	virtual OSCMessage recieve_osc() {
		std::string raw_input = this->receive_raw();
		std::cout << "Recieving " << raw_input.size() << " bytes (" << raw_input << ")\n";
		return OSCMessage::from_data(raw_input);
	};

	virtual void send_osc(OSCMessage osc) {
		std::cout << "sending message with addr = " << osc.address() << "\n";
		auto mesg = osc.to_data();

		this->send_raw(mesg);
	}

	virtual ~OSCSocketBase() = default;

protected:
	OSCSocketBase() = default;

	virtual void send_raw(std::string) = 0;
	virtual std::string receive_raw() = 0;
};

