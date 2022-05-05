#include <osc.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <charconv>

// not a fan of this - would rather have the platform take care of this, 
// but don't want to sully this with THAT yet.
// neet the endian converting functions.

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif



std::string NULL_CHAR{ "\0", 1 };

std::string pad_string(std::string padee) {
	auto padding_size = 4 - padee.size() % 4;
	for (; padding_size > 0; --padding_size) {
		padee.append(NULL_CHAR);
	}

	return padee;

}

std::string int32_to_string(long input) {
	input = htonl(input);

	// yea, could loop this, but the optimizer would then
	// have to work to unroll it. Give the poor thing a break.

	std::string retval;

	retval += (char)((input >> 24) & 0xff);
	retval += (char)((input >> 16) & 0xff);
	retval += (char)((input >> 8) & 0xff);
	retval += (char)(input & 0xff);

	return retval;
}

long string_to_int32(std::string_view input) {
	if (input.size() != 4) {
		throw std::runtime_error("input to string_to_int32 is not 4 bytes");
	}

	long retval = 0;
	retval |= long(input[0]) << 24;
	retval |= long(input[1]) << 16;
	retval |= long(input[2]) << 8;
	retval |= long(input[3]);

	return ntohl(retval);

}

std::string OSCMessage::method_name() const {
	auto pos = addr_.rfind('/');
	return addr_.substr(pos + 1);
}

void OSCMessage::add_arg(long value) {
	args_.emplace_back('i', value);
}

void OSCMessage::add_arg(std::string value) {
	args_.emplace_back('s', value);
}

void OSCMessage::add_arg(char t, std::string_view value) {
	argument new_arg;

	std::cout << "adding arg { '" << t << "', \"" << value << "\" }\n";

	switch (t) {
	case 'i':
		//new_arg.larg_ = strtol(value);
		std::from_chars(value.data(), value.data() + value.size(), new_arg.larg_);
		break;
	case 's' :
	case 'b' :
		new_arg.sarg_ = std::string(value);
		break;
	default:
		throw std::runtime_error("bad arg type");
	}
	new_arg.atype_ = t;

	args_.push_back(new_arg);
}

std::string OSCMessage::to_data() const {
	// note : currently only handles datagrams (i.e. no starting size and no SLIP)

	std::string retval = pad_string(addr_);

	std::string arg_spec = ",";
	std::string arg_val = "";

	for (auto& arg : args_) {
		arg_spec += arg.atype_;
		switch (arg.atype_) {
		case 's' :
			arg_val += pad_string(arg.sarg_);
			break;
		case 'b' :
			arg_val += int32_to_string(arg.larg_);
			arg_val += pad_string(arg.sarg_);
			break;
		case 'i' :
			arg_val += int32_to_string(arg.larg_);
			break;
		default:
			throw std::runtime_error("bad arg type");
		}

	}

	retval += pad_string(arg_spec) + arg_val;

	return retval;
}

void OSCMessage::to_stream(std::ostream& strm) const {
	strm << "path = " << address() << "\n";
	for (auto& arg : args_) {
		strm << "   " << arg.atype_ << ": " ;
		switch (arg.atype_) {
		case 'i' :
			strm << arg.larg_;
			break;
		case 's' :
		case 'b' :
			strm << '"' << arg.sarg_ << '"';
			break;
		default :
			throw std::runtime_error("ouch");
		}
		strm << "\n";
	}
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

	pos += 4 - (pos % 4);

	data.remove_prefix(pos);

	if (data[0] == ',') {
		std::cout << "Found the comma\n";

		// there are possibly arguments. IF the comma
		// is not present, we assume there is no arguments.
		// strict and grumpy about it.
		pos = data.find(NULL_CHAR);
		if (pos != std::string::npos && pos > 1) {
			auto spec = data.substr(1, pos - 1);
			data.remove_prefix(pos);
			data.remove_prefix(data.find_first_not_of(NULL_CHAR));
			for (char const atype : spec) {
				std::cout << "working on spec = '" << atype << "'\n";
				if (data.size() < 4)
					throw std::runtime_error("Ran out of data parsing args");

				argument new_arg;

				new_arg.atype_ = atype;
				switch (atype) {
				case 'i' :
					new_arg.larg_ = string_to_int32(data.substr(0, 4));
					data.remove_prefix(4);
					break;
				case 's' :
					pos = data.find(NULL_CHAR);
					std::cout << "string is " << pos << " chars long\n";
					new_arg.sarg_ = std::string(data.substr(0, pos));

					pos += 4 - (pos % 4);
					std::cout << " pos = " << pos << " length = " << data.size() << "\n";
					data.remove_prefix(pos);
					break;
				case 'b' :
					pos = string_to_int32(data.substr(0, 4));
					data.remove_prefix(4);
					new_arg.sarg_ = std::string(data.substr(0, pos));
					pos += ((pos + 1) % 4);
					data.remove_prefix(pos);
					break;
				default :
					throw std::runtime_error("Unknown arg type code in message");
					break;
				}

				msg.args_.push_back(new_arg);
			}
		}
	}
	else {
		std::cout << "No comma, so no args (we assume)\n";
	}


	return msg;

}