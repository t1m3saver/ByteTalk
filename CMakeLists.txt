cmake_minimum_required(VERSION 3.10)
project(CHATProcessor)
set(CMAKE_CXX_STANDARD 17)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(ByteTalk)
add_subdirectory(CHATCBBThirdPartyDepends)
add_subdirectory(CHATCBBCommon)
add_subdirectory(Capture)