#!/bin/bash
MAX=84
SEEDS="1234 2345 3456 4567 5678 6789"
for s in $SEEDS
do
  ./tsp --perfr $MAX --seed $s
done