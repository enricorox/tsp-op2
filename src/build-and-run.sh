#!/bin/bash
if [ -f Makefile ]; then
  make again
  echo "+------------ EXECUTABLE ------------+"
  clear
  ./tsp --file ../data/att532.tsp --time-limit 1000
  printf "\n\n"
else
  echo "Cannot find Makefile"
fi