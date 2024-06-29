cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(tokeniser)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};$ENV{LIBTORCH_DIR}")
#find_package(Torch REQUIRED)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -g")

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBXML2_CXX_FLAGS} -g")


add_executable(tokeniser src/main.cpp)
#target_link_libraries(tokeniser "${TORCH_LIBRARIES}")
target_link_libraries(tokeniser "${LIBXML2_LIBRARIES}")
set_property(TARGET tokeniser PROPERTY CXX_STANDARD 17)

#if(MSVC)
#    file(GLOB TORCH_DLLs "${TORCH_INSTALL_PREFIX}/lib/*.dll")
#    add_custom_command(TARGET mnist
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy-if-different
#        ${TORCH_DLLS})
#endif(MSVC)
