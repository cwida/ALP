#!/bin/bash

# Redirect all command output to master_script.log
exec >master_script.log 2>&1

green_echo() {
  # Print to console only
  echo -e "\033[32m-- $1\033[0m" >/dev/tty
}

green_echo "Setting up workspace variables..."
WORKSPACE=$(pwd) # Assuming this is the workspace directory
REPO_URL="https://github.com/cwida/ALP.git"
CLONED_DIR="$WORKSPACE/CLONED_ALP" # Define target directory for the clone
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
  echo -e "\033[33mPlease download the dataset from: https://drive.google.com/drive/folders/167faTwZJjqJMKM9Yc6E7KF5LUbsitxJS?usp=sharing\033[0m" >&2
  echo -e "\033[33mWarning: ALP_DATASET_DIR_PATH is not set!\033[0m" >&2
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
cmake -DALP_BUILD_PUBLICATION=ON -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -S "$CLONED_DIR" -B "$CLONED_DIR/build" -DCMAKE_BUILD_TYPE=Release -DCXX=clang++

green_echo "Building the project..."
# Build the project
cmake --build "$CLONED_DIR/build" -j 16

# End to end
{
  # Ensure the results directory exists
  RESULT_DIR="../../end_to_end_result"
  mkdir -p "$RESULT_DIR"

  # Define the output file
  OUTPUT_FILE="$RESULT_DIR/result.csv"

  # Run the main script and save output to the results file
  green_echo "Running end-to-end benchmark and saving results to $OUTPUT_FILE ..."
  export CLONED_DIR="$CLONED_DIR"
  bash "$CLONED_DIR/publication/master_script/run_end_to_end.sh" >"$OUTPUT_FILE" 2>&1

  green_echo "Benchmark completed. Results are saved in $OUTPUT_FILE."
}

green_echo "Running compression benchmarks..."
# Run compression benchmarks
"$CLONED_DIR/build/publication/source_code/bench_compression_ratio/publication_bench_alp_compression_ratio"
"$CLONED_DIR/build/publication/source_code/bench_compression_ratio/publication_bench_alp32_compression_ratio"
"$CLONED_DIR/build/publication/source_code/bench_compression_ratio/publication_bench_zstd_compression_ratio"
"$CLONED_DIR/build/publication/source_code/bench_compression_ratio/publication_bench_chimp_compression_ratio"
"$CLONED_DIR/build/publication/source_code/bench_compression_ratio/publication_bench_chimp128_compression_ratio"
"$CLONED_DIR/build/publication/source_code/bench_compression_ratio/publication_bench_gorillas_compression_ratio"
"$CLONED_DIR/build/publication/source_code/bench_compression_ratio/publication_bench_patas_compression_ratio"

green_echo "Running benchmarks based on system architecture..."
# Run benchmarks based on system architecture
if [ "$ARCH" == "arm64" ]; then
  green_echo "Running ARM64 benchmarks..."
  "$CLONED_DIR/build/publication/source_code/generated/arm64v8/neon_intrinsic_uf1/arm64v8_neon_intrinsic_1024_uf1_falp_bench"

else
  green_echo "Running x86 benchmarks..."
  "$CLONED_DIR/build/publication/source_code/generated/x86_64/avx2_intrinsic_uf1/x86_64_avx2_intrinsic_1024_uf1_falp_bench"
  "$CLONED_DIR/build/publication/source_code/generated/x86_64/avx512bw_intrinsic_uf1/x86_64_avx512bw_intrinsic_1024_uf1_falp_bench"

  green_echo "Running speed benchmarks ..."
  {
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_alp_cutter_decode"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_alp_cutter_encode"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_alp_encode"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_alp_without_sampling"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_chimp"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_chimp128"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_gorillas"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_patas"
    "$CLONED_DIR/build/publication/source_code/bench_speed/publication_bench_zstd"
  }

  green_echo "Cloning the new repository..."
  # Clone the new repository
  NEW_REPO_URL="https://github.com/azimafroozeh/bench_ped.git" # New repository URL
  NEW_TARGET_DIR="$WORKSPACE/BENCH_PED"                        # Define new target directory for the clone

  if [ -d "$NEW_TARGET_DIR" ]; then
    green_echo "New repository already exists, pulling the latest changes from branch $BRANCH..."
    cd "$NEW_TARGET_DIR" && git pull origin "$BRANCH"
  else
    green_echo "Cloning the new repository and checking out branch $BRANCH..."
    git clone --branch "$BRANCH" "$NEW_REPO_URL" "$NEW_TARGET_DIR"
  fi

  green_echo "Navigating to new target directory..."
  # shellcheck disable=SC2164
  cd "$NEW_TARGET_DIR"

  green_echo "Creating build directory for the new repository..."
  # Create build directory
  mkdir -p "$NEW_TARGET_DIR/build"

  green_echo "Configuring CMake for the new repository..."
  # Configure CMake for the new repository
  cmake -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -S "$NEW_TARGET_DIR" -B "$NEW_TARGET_DIR/build" -DCMAKE_BUILD_TYPE=Release -DCXX=clang++

  green_echo "Building the project for the new repository..."
  # Build the project for the new repository
  cmake --build "$NEW_TARGET_DIR/build" -j 16

  green_echo "Running specific targets for the new repository..."
  # Run specific targets
  cmake --build "$NEW_TARGET_DIR/build" --target bench_ped -j 16
  cmake --build "$NEW_TARGET_DIR/build" --target test_ped -j 16

  green_echo "Executing the new targets..."
  # Execute the new targets
  "$NEW_TARGET_DIR/build/bench_ped"
  "$NEW_TARGET_DIR/build/test_ped"
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
