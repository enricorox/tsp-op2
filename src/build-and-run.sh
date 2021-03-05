#!/bin/bash
if [ -f Makefile ]; then
  make again
  echo "+------------ EXECUTABLE ------------+"
  clear
  ./tsp
  printf "\n\n"
else
  echo "Cannot find Makefile"
fi