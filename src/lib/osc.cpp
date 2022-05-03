#include <osc.hpp>
#include <iostream>
#include <string>

std::string NULL_CHAR{ "\0", 1 };

std::string pad_string(std::string padee) {
	auto padding_size = padee.size() % 4;
	for (; padding_size > 0; --padding_size) {
		padee.append(NULL_CHAR);
	}

	return padee;

}

std::string OSCMessage::method_name() {
	auto pos = addr_.rfind('/');
	return addr_.substr(pos + 1);
}

std::string OSCMessage::to_data() {
	// note : currently only handles datagrams (i.e. no starting size and no SLIP)

	std::string retval = addr_;

	// OSC string are null terminated. This needs to be done BEFORE the padding.
	retval += NULL_CHAR;

	retval = pad_string(retval) + pad_string("," + NULL_CHAR);

	return retval;
}

OSCMessage OSCMessage::from_data(std::string_view data) {
	// note : currently only handles datagrams (i.e. no starting size and no SLIP)
	// per examples at https://opensoundcontrol.stanford.edu/spec-1_0-examples.html
	// The address pattern is also padded to a 4 byte boundary.

	OSCMessage msg;

	// There will always be a null at the end of the path. Find it.
	auto pos = data.find(NULL_CHAR);
	std::cout << "addr ends at pos = " << pos << "\n";

	auto new_addr = data.substr(0, pos);

	// force the copy of the underlying bits.
	msg.addr_ = std::string(new_addr);

	return msg;

}