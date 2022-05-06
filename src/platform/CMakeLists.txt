#
# OSCAppLauncherService/platform
#
cmake_minimum_required(VERSION 3.18)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(PLATFORM_DIR "win")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_DIR "linux")
else()
    message(FATAL_ERROR "unknown platform ${CMAKE_SYSTEM_NAME}")
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
