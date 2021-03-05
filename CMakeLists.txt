cmake_minimum_required(VERSION 3.10.2)
project(tsp_op2 C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(/opt/ibm/ILOG/CPLEX_Studio201/cplex/include/ilcplex)
link_directories(/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic)

add_executable(tsp
        src/main.c
        src/tsp.c
        src/tsp.h)

target_link_libraries(tsp cplex)