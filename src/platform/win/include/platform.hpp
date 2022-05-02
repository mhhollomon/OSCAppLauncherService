//
// Windows platform
//

#include <platform_base.hpp>

class Platform : public PlatformBase {
public:
	virtual std::string get_ini_file_name();

};