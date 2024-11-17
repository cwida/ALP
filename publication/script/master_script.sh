#!/bin/bash

# Redirect all command output to master_script.log
exec >master_script.log 2>&1

# Define colored output functions
green_echo() {
  echo -e "\033[32m-- $1\033[0m" >/dev/tty
}

brown_echo() {
  echo -e "\033[33m $1\033[0m" >/dev/tty
}

black_echo() {
  echo -e "\033[30m $1\033[0m" >/dev/tty
}

red_echo() {
  echo -e "\033[31mError: $1\033[0m" >/dev/tty
}

# Set up workspace variables
green_echo "Setting up workspace variables..."
WORKSPACE=$(pwd)
REPO_URL="https://github.com/cwida/ALP.git"
CLONED_DIR="$WORKSPACE/ALP"
BRANCH="repro"

# Clone or update the repository
green_echo "Cloning or updating repository..."
if [ -d "$CLONED_DIR" ]; then
  green_echo "Repository already exists. Checking for the latest changes..."
  cd "$CLONED_DIR"
  git fetch origin "$BRANCH"
  git_status=$(git status -uno)
  if echo "$git_status" | grep -q "up to date"; then
    green_echo "Repository is already up to date."
  else
    green_echo "Pulling the latest changes from branch $BRANCH..."
    git pull origin "$BRANCH"
    if [ $? -ne 0 ]; then
      red_echo "Failed to pull the latest changes from branch $BRANCH."
      exit 1
    fi
  fi
else
  green_echo "Cloning the repository and checking out branch $BRANCH..."
  git clone --branch "$BRANCH" "$REPO_URL" "$CLONED_DIR"
  if [ $? -ne 0 ]; then
    red_echo "Failed to clone the repository."
    exit 1
  fi
fi

# Navigate to the repository
green_echo "Navigating to target directory..."
cd "$CLONED_DIR"

# Create build directory
green_echo "Creating build directory..."
mkdir -p "$CLONED_DIR/build"

# Check if ALP_DATASET_DIR_PATH is set
green_echo "Checking if ALP_DATASET_DIR_PATH is set..."
if [ -z "$ALP_DATASET_DIR_PATH" ]; then
  red_echo "Please download the dataset from: https://drive.google.com/drive/folders/167faTwZJjqJMKM9Yc6E7KF5LUbsitxJS?usp=sharing"
  red_echo "Warning: ALP_DATASET_DIR_PATH is not set!"
  exit 1
else
  green_echo "ALP_DATASET_DIR_PATH is set to $ALP_DATASET_DIR_PATH"
fi

# Determine system architecture and platform
green_echo "Determining system architecture and platform..."
ARCH=$(uname -m)
PLATFORM=$(uname -s)

if [ "$ARCH" == "arm64" ] && [ "$PLATFORM" == "Darwin" ]; then
  TOOLCHAIN_FILE="$CLONED_DIR/toolchain/m1.cmake"
  green_echo "Using M1 toolchain file..."
elif [ "$ARCH" == "arm64" ]; then
  TOOLCHAIN_FILE="$CLONED_DIR/toolchain/arm64.cmake"
  green_echo "Using ARM64 toolchain file..."
else
  TOOLCHAIN_FILE="$CLONED_DIR/toolchain/x86.cmake"
  green_echo "Using x86 toolchain file..."
fi

# Configure CMake
green_echo "Configuring CMake..."
cmake -DALP_BUILD_PUBLICATION=ON -DALP_ENABLE_VERBOSE_OUTPUT=ON -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -S "$CLONED_DIR" -B "$CLONED_DIR/build" -DCMAKE_BUILD_TYPE=Release -DCXX=clang++
if [ $? -ne 0 ]; then
  red_echo "CMake configuration failed."
  exit 1
fi

# Build the project
green_echo "Building the project..."
cmake --build "$CLONED_DIR/build" -j 6
if [ $? -ne 0 ]; then
  red_echo "CMake build failed."
  exit 1
fi

# Generate compression ratio tables
green_echo "Generating compression ratio tables..."
output=$(python3 "$CLONED_DIR/publication/master_script/generate_tables.py")
black_echo "$output"

# Run benchmarks based on system architecture
green_echo "Running benchmarks based on system architecture..."
if [ "$ARCH" == "arm64" ]; then
  green_echo "Running ARM64 benchmarks..."
  "$CLONED_DIR/build/publication/source_code/generated/arm64v8/neon_intrinsic_uf1/arm64v8_neon_intrinsic_1024_uf1_falp_bench"
else
  # End-to-end benchmark
  SCRIPT_DIR=$(dirname "$(realpath "$0")")
  OUTPUT_FILE="$SCRIPT_DIR/end_to_end_result.csv"
  green_echo "Running end-to-end benchmark and saving results to $OUTPUT_FILE ..."
  export CLONED_DIR="$CLONED_DIR"
  bash "$CLONED_DIR/publication/master_script/run_end_to_end.sh" >"$OUTPUT_FILE" 2>&1
  green_echo "Benchmark completed. Results are saved in $OUTPUT_FILE."

  # Clone and build the BENCH_PED repository
  green_echo "Cloning the BENCH_PED repository..."
  NEW_REPO_URL="https://github.com/azimafroozeh/bench_ped.git"
  PED_DIR="$WORKSPACE/BENCH_PED"
  BRANCH="main"

  if [ -d "$PED_DIR" ]; then
    green_echo "Repository already exists. Pulling the latest changes..."
    cd "$PED_DIR"
    git fetch origin "$BRANCH"
    git_status=$(git status -uno)
    if echo "$git_status" | grep -q "up to date"; then
      green_echo "Repository is already up to date."
    else
      green_echo "Pulling the latest changes from branch $BRANCH..."
      git pull origin "$BRANCH"
      if [ $? -ne 0 ]; then
        red_echo "Failed to pull the latest changes."
        exit 1
      fi
    fi
  else
    green_echo "Cloning the repository..."
    git clone --branch "$BRANCH" "$NEW_REPO_URL" "$PED_DIR"
    if [ $? -ne 0 ]; then
      red_echo "Failed to clone the repository."
      exit 1
    fi
  fi

  # Navigate to PED directory and build
  green_echo "Building the BENCH_PED repository..."
  cd "$PED_DIR"
  mkdir -p build
  cmake -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -S "$PED_DIR" -B "$PED_DIR/build" -DCMAKE_BUILD_TYPE=Release -DCXX=clang++
  if [ $? -ne 0 ]; then
    red_echo "CMake configuration failed for BENCH_PED."
    exit 1
  fi
  cmake --build "$PED_DIR/build" -j 6
  if [ $? -ne 0 ]; then
    red_echo "Build failed for BENCH_PED."
    exit 1
  fi
fi

# Set up Python and install dependencies
green_echo "Setting up Python and installing dependencies..."
python -m pip install --upgrade pip
pip install -r "$CLONED_DIR/publication/plotter/requirements.txt"

# Run the plotter script
green_echo "Running the plotter script..."
python3 "$CLONED_DIR/publication/plotter/plotter.py"

green_echo "Script execution complete. All logs saved to master_script.log."
