#
# OSCAppLauncherService/src
#

cmake_minimum_required(VERSION 3.18)

##
## lib-include
##
add_library(lib_include INTERFACE)

target_include_directories(lib_include
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )

##
## lib-src
##
add_library(lib_src INTERFACE)

target_include_directories(lib_src
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    )
target_link_libraries(lib_src
    INTERFACE lib_include
    )

##
## osc_objlib
##
add_library(osc_objlib OBJECT)

target_sources(osc_objlib
    PRIVATE
        "lib/osc.cpp"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/osc.hpp"
    )

target_link_libraries(osc_objlib 
    PUBLIC
        lib_include
    )

##
## config_objlib
##
add_library(config_objlib OBJECT)

target_sources(config_objlib
    PRIVATE
        "lib/config.cpp"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/config.hpp"
    )

target_link_libraries(config_objlib
    PRIVATE
    	${config_lib}

    PUBLIC
        lib_include
    )

# this needs to come last so the platform can use stuff in lib if needed.
add_subdirectory(platform)
