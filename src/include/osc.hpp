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

		template<class T> auto get() -> T {

			if constexpr ( std::is_integral_v<T> ) {
				if (atype_ == 'i') {
					return larg_;

				}
				else {
					throw std::runtime_error("Not an integer argument type");
				}
			} else {
				throw std::runtime_error("unknown type");
			}
		}
		template<> auto get<std::string>() -> std::string {
			if ((atype_ == 's') || (atype_ = 'b')) {
				return sarg_;
			}
			else {
				throw std::runtime_error("Not a string argument type");
			}
		}

		template<> auto get<long>() -> long {
			if (atype_ == 'i') {
				return larg_;
			}
			else {
				throw std::runtime_error("Not a integral argument type");
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

	OSCMessage(std::string addr) { 
		addr_ = addr;
	}

	static OSCMessage from_data(std::string_view data);

private :
	OSCMessage() = default;

};
