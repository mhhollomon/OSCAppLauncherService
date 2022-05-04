#
# OSCAppLauncherService/extern
#
cmake_minimum_required(VERSION 3.18)

include(FetchContent)

add_definitions(-DLIBCONFIG_STATIC)

#add_library(inipp INTERFACE)
#target_include_directories(inipp INTERFACE inipp/)

FetchContent_Declare(
  libconfig
  GIT_REPOSITORY https://github.com/mhhollomon/libconfig.git
  GIT_TAG 04c5f9a8b3f8623ec0a4f61bfd5dbddb817997ed
)

FetchContent_MakeAvailable( libconfig )
