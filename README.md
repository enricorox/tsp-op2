# Travelling Salesman (Salesperson) Problem
Homeworks from Operational Research 2 at University of Padua
## Requirements
Install [IMB CPLEX Optimizer](https://www.ibm.com/analytics/cplex-optimizer).
## How to compile
1. Set your own `CPLEX_HEADERS` and `CLEX_LIBS` path on `CMakeLists.txt`
2. Run `cmake .`
3. Run `make`
## How to run
Type `./tsp --help` to see available parameters.  
Mandatory parameters are:
1. `--file <file>` specify tsp file path 
2. `--time-limit <time>` specify overall time in seconds
## Data
You can download tsp input files from [here](http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/tsp/).