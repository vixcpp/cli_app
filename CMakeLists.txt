cmake_minimum_required(VERSION 3.20)
project(cli_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cli_app INTERFACE)
add_library(cli_app::cli_app ALIAS cli_app)

target_include_directories(cli_app INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(cli_app INTERFACE /W4 /permissive-)
else()
  target_compile_options(cli_app INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(.vix/vix_deps.cmake)
target_link_libraries(cli_app INTERFACE vix::app)

include(CTest)
enable_testing()

add_executable(cli_app_basic_test tests/test_basic.cpp)
target_link_libraries(cli_app_basic_test PRIVATE cli_app::cli_app)
add_test(NAME cli_app.basic COMMAND cli_app_basic_test)
