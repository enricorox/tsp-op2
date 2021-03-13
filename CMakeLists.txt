cmake_minimum_required(VERSION 3.10.2)
project(tsp_op2 C)

set(CMAKE_C_STANDARD 11)

# change path if needed
set(CPLEX_HEADERS "/opt/ibm/ILOG/CPLEX_Studio201/cplex/include/ilcplex/")
set(CPLEX_LIBS_STATIC "/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic/")

include_directories(src)
include_directories(${CPLEX_HEADERS})
link_directories(${CPLEX_LIBS_STATIC})

add_executable(tsp
        src/main.c
        src/tsp.c
        src/tsp.h src/plot.c src/plot.h)

target_link_libraries(tsp cplex m pthread dl)