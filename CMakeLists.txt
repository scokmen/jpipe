cmake_minimum_required(VERSION 3.30)
project(jpipe C VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

set(SOURCES 
        src/main.c
)

add_executable(jpipe ${SOURCES})
