project(libsimulator)
cmake_minimum_required(VERSION 2.8.7)

set(SRC_DIR src)
set(TEST_SRC_DIR test)
set(INCLUDE_DIR include)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)

set(SIMULATOR_SRC_FILES
  ${SRC_DIR}/acceptor.cpp
  ${SRC_DIR}/high_resolution_timer.cpp
  ${SRC_DIR}/io_service.cpp
  ${SRC_DIR}/resolver.cpp
  ${SRC_DIR}/sink_forwarder.cpp
  ${SRC_DIR}/udp_socket.cpp
  ${SRC_DIR}/default_config.cpp
  ${SRC_DIR}/http_proxy.cpp
  ${SRC_DIR}/pcap.cpp
  ${SRC_DIR}/simulation.cpp
  ${SRC_DIR}/socks_server.cpp
  ${SRC_DIR}/high_resolution_clock.cpp
  ${SRC_DIR}/http_server.cpp
  ${SRC_DIR}/queue.cpp
  ${SRC_DIR}/simulator.cpp
  ${SRC_DIR}/tcp_socket.cpp
  ${SRC_DIR}/nat.cpp
)

add_library(simulator ${SIMULATOR_SRC_FILES})
include_directories(${Boost_INCLUDE_DIR} ${INCLUDE_DIR})

if(WIN32)
  target_link_libraries(simulator PRIVATE ws2_32)
endif()

enable_testing()

function(define_test)
  set(NAME ${ARGV0})
  add_executable(${NAME} ${TEST_SRC_DIR}/${NAME}.cpp ${TEST_SRC_DIR}/main.cpp)
  target_link_libraries(${NAME} simulator ${Boost_LIBRARIES})
  target_link_libraries(${NAME} Threads::Threads)
  add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

define_test(acceptor)
define_test(multi_accept)
define_test(multi_homed)
define_test(null_buffers)
define_test(parse_request)
define_test(resolver)
define_test(timer)
define_test(udp_socket)
