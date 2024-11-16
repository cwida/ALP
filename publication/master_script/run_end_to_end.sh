#!/bin/bash

#declare -a datasets=("food_prices" "city_temperature_f" "bitcoin_transactions_f" "gov26" "nyc29")
#declare -a schemes=("alp" "pde" "patas" "gorilla" "alp_rd" "ztsd" "chimp" "chimp128")
#declare -a queries=("scan" "sum")
#declare -a threads=("1" "8" "16")

declare -a datasets=("food_prices")
declare -a schemes=("alp" )
declare -a queries=("scan" )
declare -a threads=("1")

for scheme in "${schemes[@]}"; do
  for query in "${queries[@]}"; do
    for thread in "${threads[@]}"; do
      for dataset in "${datasets[@]}"; do
        /home/ubuntu/alp/cmake-build-release-i4i/publication/source_code/bench_end_to_end/run_query "$thread" "$query" "$scheme" "$dataset"
      done
    done
  done
done
