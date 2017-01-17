cmake_minimum_required(VERSION 2.6)
project(redemption)

add_executable(redemption main.cpp)

install(TARGETS redemption RUNTIME DESTINATION bin)
