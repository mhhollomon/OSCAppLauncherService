#
# OSCAppLauncherService
#
cmake_minimum_required (VERSION 3.18)

project ("OSCAppLauncherService" VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)


if(MSVC)
    set(config_lib "libconfig++")
else()
    set(config_lib "config++")
endif()

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
		${config_lib}
		simpleConfig
		osc_objlib
		config_objlib
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
		osc_objlib
		platform_objlib
	)

########## OALMidi2osc

#add_executable(OALMidi2osc)

#target_sources(OALMidi2osc
#	PRIVATE
#		"src/OALMidi2osc.cpp"
#	)

#target_link_libraries(OALMidi2osc
#    PRIVATE
#        lib_include
#		${config_lib}
#		simpleConfig
#		osc_objlib
#		config_objlib
#		platform_objlib
#	)
# TODO: Add tests and install targets if needed.
