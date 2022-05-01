#include <osc.hpp>
#include <iostream>
#include <string>

std::string OSCMessage::method_name() {
	auto pos = addr_.rfind('/');
	return addr_.substr(pos + 1);
}

std::string OSCMessage::to_data() {
	// note : currently only handles datagrams (i.e. no starting size and no SLIP)

	std::string retval = addr_;

	auto padding_size = addr_.size() % 4;

	std::string null_char{ "\0", 1 };
	for (; padding_size > 0; --padding_size) {
		std::cout << "Appending null";
		retval.append(null_char);
	}
	retval.append(",");

	return retval;
}

OSCMessage OSCMessage::from_data(std::string_view data) {
	// note : currently only handles datagrams (i.e. no starting size and no SLIP)
	// per examples at https://opensoundcontrol.stanford.edu/spec-1_0-examples.html
	// The address pattern is also padded to a 4 bit boundary.

	OSCMessage msg;

	// We are going to assume that it includes a type tag.
	auto type_tag_pos = data.find(',');
	std::cout << "tag pos = " << type_tag_pos << "\n";
	auto new_addr = data.substr(0, type_tag_pos);
	auto pos = data.find('\0');
	std::cout << "pos = " << pos << "\n";

	if (pos != std::string::npos) {
		std::cout << "Stripping nulls\n";
		new_addr = new_addr.substr(0, pos);
	}

	// force the copy of the underlying bits.
	msg.addr_ = std::string(new_addr);

	return msg;

}