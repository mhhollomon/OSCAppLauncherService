#
# OSCAppLauncherService/extern
#
cmake_minimum_required(VERSION 3.18)

include(FetchContent)

add_definitions(-DLIBCONFIG_STATIC)

FetchContent_Declare(
  libconfig
  GIT_REPOSITORY https://github.com/hyperrealm/libconfig.git
  GIT_TAG f9404f60a435aa06321f4ccd8357364dcb216d46
)

FetchContent_MakeAvailable( libconfig )

FetchContent_Declare(
  simpleConfig
  GIT_REPOSITORY https://github.com/mhhollomon/simpleConfig.git
  GIT_TAG v0.2.0
)

FetchContent_MakeAvailable( simpleConfig )
