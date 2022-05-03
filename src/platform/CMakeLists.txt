#
# OSCAppLauncherService/platform
#
cmake_minimum_required(VERSION 3.20)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(PLATFORM_DIR "win")
else()
	message(FATAL_ERROR "unknown platform")
endif()

##
## platform-include
##
add_library(platform_include INTERFACE)

target_include_directories(platform_include
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_DIR}/include"
    )


##
## platform_objlib
##
add_library(platform_objlib OBJECT)

target_sources(platform_objlib
    PRIVATE
        "${PLATFORM_DIR}/lib/platform.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_DIR}/include/osc_socket.hpp"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_DIR}/include/platform.hpp"
    )

target_link_libraries(platform_objlib 
    PUBLIC
        platform_include
        lib_include
    )
