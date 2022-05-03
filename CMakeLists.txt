#
# OSCAppLauncherService
#
cmake_minimum_required (VERSION 3.20)

project ("OSCAppLauncherService" VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(extern)

add_subdirectory(src)



########## OALServer
add_executable (OALServer)

target_sources(OALServer
	PRIVATE
		"src/OALServer.cpp" 
)

target_link_libraries(OALServer
    PRIVATE
        lib_include
		inipp
		osc_objlib
		platform_objlib
	)

########## OALSend

add_executable(OALSend)

target_sources(OALSend
	PRIVATE
		"src/OALSend.cpp"
	)

target_link_libraries(OALSend
    PRIVATE
        lib_include
		inipp
		osc_objlib
		platform_objlib
	)

# TODO: Add tests and install targets if needed.
