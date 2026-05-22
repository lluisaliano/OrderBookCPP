cmake_minimum_required(VERSION 4.0)

project(OrderBook VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(orderbook src/main.cpp)

target_compile_options(orderbook PRIVATE
  -Wall -Wextra -Wpedantic -Wsign-conversion
)
target_include_directories(orderbook PUBLIC ${PROJECT_SOURCE_DIR}/include)
