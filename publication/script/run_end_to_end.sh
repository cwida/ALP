#!/bin/bash

declare -a datasets=("food_prices" "city_temperature_f" "bitcoin_transactions_f" "gov26" "nyc29")
declare -a schemes=("ztsd" "chimp" "chimp128")
declare -a queries=("scan" "sum")
declare -a threads=("1" "8" "16")

#declare -a schemes=("alp" "pde" "patas" "gorilla" "ztsd" "chimp" "chimp128")

#declare -a datasets=("food_prices")
#declare -a schemes=("alp")
#declare -a queries=("scan")
#declare -a threads=("1")

# Ensure CLONED_DIR is set
if [ -z "$CLONED_DIR" ]; then
  echo "Error: CLONED_DIR is not set. Please set it before running this script."
  exit 1
fi

# Iterate through combinations
for scheme in "${schemes[@]}"; do
  for query in "${queries[@]}"; do
    for thread in "${threads[@]}"; do
      for dataset in "${datasets[@]}"; do
        # Skip specific combination
        if [[ "$scheme" == "pde" && "$dataset" == "nyc29" ]]; then
          continue
        fi

        # Execute the command
        "$CLONED_DIR/build/publication/source_code/bench_end_to_end/run_query" "$thread" "$query" "$scheme" "$dataset"

        # Check for errors
        if [ $? -ne 0 ]; then
          echo "Error: Command failed for dataset='$dataset', scheme='$scheme', query='$query', thread='$thread'."
          exit 1
        fi
      done
    done
  done
done
