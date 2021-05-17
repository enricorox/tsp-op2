cmake_minimum_required(VERSION 3.10.2)
project(tsp_op2 C)

set(CMAKE_C_STANDARD 11)

# change path if needed
set(CPLEX_HEADERS "/opt/ibm/ILOG/CPLEX_Studio201/cplex/include/ilcplex/")
set(CPLEX_LIBS_STATIC "/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic/")
set(CONCORDE_HEADER "/opt/concorde/")
set(CONCORDE_LIB "/opt/concorde/")

include_directories(src)

include_directories(${CPLEX_HEADERS})
include_directories(${CONCORDE_HEADER})

link_directories(${CPLEX_LIBS_STATIC})
link_directories(${CONCORDE_LIB})

add_executable(tsp
        src/main.c
        src/tsp.c src/tsp.h
        src/plot.c src/plot.h
        src/utils.c src/utils.h
        src/formulation_Benders.c src/formulation_Benders.h
        src/formulation_MTZ.c src/formulation_MTZ.h
        src/formulation_commons.c src/formulation_commons.h
        src/formulation_GG.c src/formulation_GG.h
        src/performance.c src/performance.h
        src/distances.c src/distances.h
        src/parsers.c src/parsers.h
        src/formulation_cuts.c src/formulation_cuts.h
        src/formulation_sfixing.c src/formulation_sfixing.h
        src/heuristics.c src/heuristics.h
        src/heuristic_greedy.c src/heuristic_greedy.h
        src/heuristic_extramileage.c src/heuristic_extramileage.h
        src/graham_scan.c src/graham_scan.h src/heuristic_kopt.c src/heuristic_kopt.h)

target_link_libraries(tsp cplex m pthread dl)