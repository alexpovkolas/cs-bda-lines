cmake_minimum_required(VERSION 3.12)
project(cs_bda_lines)

set(CMAKE_CXX_STANDARD 14)
find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)

SET(CMAKE_C_FLAGS -Drestrict=__restrict__)
SET(CMAKE_C_FLAGS -O3)
SET(CMAKE_LINK_DEF_FILE_FLAG -lpthread)

add_executable(cs_bda_lines main.cpp)