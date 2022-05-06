#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <ostream>
#include <type_traits>

class OSCMessage {

	struct argument {
		char atype_ = 'x';
		long larg_;
		std::string sarg_;

		argument() = default;

		argument(char t, long v) : atype_(t), larg_(v) {}

		argument(char t, std::string v) : atype_(t), sarg_(v) {}

		bool isString() const {
			return ((atype_ == 's') || (atype_ == 'b'));
		}

		bool isInt() const {
			return (atype_ == 'i');
		}

		bool isBlob() const {
			return (atype_ == 'b');
		}

		template<class T> auto get() const -> T  {

			if constexpr ( std::is_integral_v<T> ) {
				if (isInt()) {
					return larg_;

				}
				else {
					throw std::runtime_error("Not an integer argument type");
				}
            } else if constexpr (std::is_same_v<std::string, T>) {

                if (isString()) {
                    return sarg_;
                }
                else {
                    throw std::runtime_error("Not a string argument type");
                }
            } else {
                throw std::runtime_error("unknown type");
            }
		}

	};
	std::string addr_;
	std::vector<argument> args_;
public:
	std::string method_name() const;
	std::string address() const { return addr_; };

	bool hierarchy_match(std::string path) const {
		return (addr_.rfind(path + "/", 0) == 0);
	}

	std::string to_data() const;

	void to_stream(std::ostream&) const;

	void add_arg(long);
	void add_arg(std::string);
	void add_arg(char, std::string_view);

	int arg_count() const {
		return args_.size();
	}

	const argument& get_arg(int index) {
		if (index > -1 && index <= (arg_count() - 1)) {
			return args_[index];
		}
		else {
			throw std::runtime_error("Index out of range");
		}
	}

	OSCMessage(std::string addr) { 
		addr_ = addr;
	}

	static OSCMessage from_data(std::string_view data);

private :
	OSCMessage() = default;

};
