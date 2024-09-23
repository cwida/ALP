#!/bin/bash

# check BENCHMARKING.md for more details.

WORKSPACE=$(pwd)  # Assuming this is the workspace directory
REPO_URL="https://github.com/azimafroozeh/_ALP.git"
TARGET_DIR="$WORKSPACE/ALP"  # Define target directory for the clone
BRANCH="2_ari"  # Branch to clone

# Clone the repository if it doesn't already exist
if [ -d "$TARGET_DIR" ]; then
  echo "Repository already exists, pulling the latest changes from branch $BRANCH..."
  cd "$TARGET_DIR" && git pull origin "$BRANCH"
else
  echo "Cloning the repository and checking out branch $BRANCH..."
  git clone --branch "$BRANCH" "$REPO_URL" "$TARGET_DIR"
fi

# Move to the cloned repository
# shellcheck disable=SC2164
cd "$TARGET_DIR"

# Create build directory
mkdir -p "$TARGET_DIR/build"

# Configure CMake
cmake -DALP_BUILD_PUBLICATION=ON -DCMAKE_TOOLCHAIN_FILE="$TARGET_DIR/toolchain/example.cmake" -S "$TARGET_DIR" -B "$TARGET_DIR/build" -DCMAKE_BUILD_TYPE=Release -DCXX=clang++

# Build the project
cmake --build "$TARGET_DIR/build" -j 16

# Run tests
#cd "$TARGET_DIR/build" && ctest -j 4  todo

# Check if ALP_DATASET_DIR_PATH is set
# Set the environment variable `ALP_DATASET_DIR_PATH` with the path to the directory in which the complete
# binary datasets are located
if [ -z "$ALP_DATASET_DIR_PATH" ]; then
  echo -e "\033[33mPlease download the dataset from: https://drive.google.com/drive/folders/167faTwZJjqJMKM9Yc6E7KF5LUbsitxJS?usp=sharing\033[0m"
  echo -e "\033[33mWarning: ALP_DATASET_DIR_PATH is not set!\033[0m"
  exit 1
else
  echo "ALP_DATASET_DIR_PATH is set to $ALP_DATASET_DIR_PATH"
fi

# Run benchmarks
#"$TARGET_DIR/build/publication/source_code/bench_compression_ratio/bench_alp_compression_ratio"
#"$TARGET_DIR/build/publication/source_code/bench_compression_ratio/bench_alp32_compression_ratio"
#"$TARGET_DIR/build/publication/source_code/bench_compression_ratio/bench_zstd_compression_ratio"
#"$TARGET_DIR/build/publication/source_code/bench_compression_ratio/bench_chimp_compression_ratio"
#"$TARGET_DIR/build/publication/source_code/bench_compression_ratio/bench_chimp128_compression_ratio"
#"$TARGET_DIR/build/publication/source_code/bench_compression_ratio/bench_gorillas_compression_ratio"
#"$TARGET_DIR/build/publication/source_code/bench_compression_ratio/bench_patas_compression_ratio"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_alp_cutter_decode"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_alp_cutter_encode"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_alp_encode"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_alp_without_sampling"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_chimp"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_chimp128"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_gorillas"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_patas"
"$TARGET_DIR/build/publication/source_code/bench_speed/bench_zstd"

# Set up Python and install dependencies
python -m pip install --upgrade pip
pip install -r "$TARGET_DIR/publication/plotter/requirements.txt"

# Run the plotter script
python "$TARGET_DIR/publication/plotter/plotter.py"
