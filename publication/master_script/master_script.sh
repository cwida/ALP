#!/bin/bash

# Redirect all command output to master_script.log
exec >master_script.log 2>&1

green_echo() {
  # Print to console only
  echo -e "\033[32m-- $1\033[0m" >/dev/tty
}

brown_echo() {
  # Print to console only in brown (dark yellow)
  echo -e "\033[33m $1\033[0m" >/dev/tty
}

red_echo() {
  # Print to console only in red
  echo -e "\033[31mError: $1\033[0m" >/dev/tty
}

green_echo "Setting up workspace variables..."
WORKSPACE=$(pwd) # Assuming this is the workspace directory
REPO_URL="https://github.com/cwida/ALP.git"
CLONED_DIR="$WORKSPACE/ALP" # Define target directory for the clone
BRANCH="repro"              # Branch to clone

green_echo "Cloning or updating repository..."
# Clone the repository if it doesn't already exist
if [ -d "$CLONED_DIR" ]; then
  green_echo "Repository already exists, pulling the latest changes from branch $BRANCH..."
  cd "$CLONED_DIR" && git pull origin "$BRANCH"
else
  green_echo "Cloning the repository and checking out branch $BRANCH..."
  git clone --branch "$BRANCH" "$REPO_URL" "$CLONED_DIR"
fi

# Move to the cloned repository
green_echo "Navigating to target directory..."
cd "$CLONED_DIR"

# Create build directory
green_echo "Creating build directory..."
mkdir -p "$CLONED_DIR/build"

green_echo "Checking if ALP_DATASET_DIR_PATH is set..."
# Check if ALP_DATASET_DIR_PATH is set
if [ -z "$ALP_DATASET_DIR_PATH" ]; then
  red_echo "Please download the dataset from: https://drive.google.com/drive/folders/167faTwZJjqJMKM9Yc6E7KF5LUbsitxJS?usp=sharing"
  red_echo "Warning: ALP_DATASET_DIR_PATH is not set!"
  exit 1
else
  green_echo "ALP_DATASET_DIR_PATH is set to $ALP_DATASET_DIR_PATH"
fi

green_echo "Determining system architecture and platform..."
# Determine system architecture and platform
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

green_echo "Configuring CMake..."
# Configure CMake with the selected toolchain file
cmake -DALP_BUILD_PUBLICATION=ON -DALP_ENABLE_VERBOSE_OUTPUT=ON -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -S "$CLONED_DIR" -B "$CLONED_DIR/build" -DCMAKE_BUILD_TYPE=Release -DCXX=clang++
if [ $? -ne 0 ]; then
  red_echo "CMake configuration failed."
  exit 1
fi

green_echo "Building the project..."
# Build the project
cmake --build "$CLONED_DIR/build" -j 6
if [ $? -ne 0 ]; then
  red_echo "CMake build failed."
  exit 1
fi

green_echo "Generating compression ratio table 4 ..."
output=$(python3 "$CLONED_DIR/publication/master_script/draw_table_4.py")
brown_echo "$output"
green_echo "Table 4 also saved as compression_ratios_table.md."

green_echo "Running benchmarks based on system architecture..."
# Run benchmarks based on system architecture
if [ "$ARCH" == "arm64" ]; then
  green_echo "Running ARM64 benchmarks..."
  "$CLONED_DIR/build/publication/source_code/generated/arm64v8/neon_intrinsic_uf1/arm64v8_neon_intrinsic_1024_uf1_falp_bench"

else
  # End to end
  OUTPUT_FILE="end_to_end_result.csv"
  green_echo "Running end-to-end benchmark and saving results to $OUTPUT_FILE ..."
  export CLONED_DIR="$CLONED_DIR"
  bash "$CLONED_DIR/publication/master_script/run_end_to_end.sh" >"$OUTPUT_FILE" 2>&1

  green_echo "Benchmark completed. Results are saved in $OUTPUT_FILE."

  green_echo "Cloning the BENCH_PED ..."
  # Clone the new repository
  NEW_REPO_URL="https://github.com/azimafroozeh/bench_ped.git" # New repository URL
  PED_DIR="$WORKSPACE/BENCH_PED"                               # Define new target directory for the clone
  BRANCH="main"

  if [ -d "$PED_DIR" ]; then
    green_echo "New repository already exists, pulling the latest changes from branch $BRANCH..."
    cd "$PED_DIR" && git pull origin "$BRANCH"
  else
    green_echo "Cloning the new repository and checking out branch $BRANCH..."
    git clone --branch "$BRANCH" "$NEW_REPO_URL" "$PED_DIR"
  fi

  green_echo "Navigating to new target directory..."
  cd "$PED_DIR"

  green_echo "Creating build directory for the new repository..."
  # Create build directory
  mkdir -p "$PED_DIR/build"

  green_echo "Configuring CMake for the new repository..."
  # Configure CMake for the new repository
  cmake -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -S "$PED_DIR" -B "$PED_DIR/build" -DCMAKE_BUILD_TYPE=Release -DCXX=clang++
  if [ $? -ne 0 ]; then
    red_echo "CMake configuration for the new repository failed."
    exit 1
  fi

  green_echo "Running specific targets for the new repository..."
  # Run specific targets
  cmake --build "$PED_DIR/build" --target bench_ped -j 1
  if [ $? -ne 0 ]; then
    red_echo "Target bench_ped failed."
    exit 1
  fi

  cmake --build "$PED_DIR/build" --target test_ped -j 1
  if [ $? -ne 0 ]; then
    red_echo "Target test_ped failed."
    exit 1
  fi

  green_echo "Executing the new targets..."
  # Execute the new targets
  "$PED_DIR/build/bench_ped"
  "$PED_DIR/build/test_ped"
fi

green_echo "Setting up Python and installing dependencies..."
# Set up Python and install dependencies
{
  python -m pip install --upgrade pip
  pip install -r "$CLONED_DIR/publication/plotter/requirements.txt"
}

green_echo "Running the plotter script..."
# Run the plotter script
python3 "$CLONED_DIR/publication/plotter/plotter.py"

green_echo "Script execution complete. All logs saved to master_script.log."
