#!/bin/bash

# Update package lists
echo "Updating package lists..."
sudo apt-get update

# Install Clang++
echo "Installing Clang++..."
sudo apt-get install -y clang

# Install CMake (version 3.22 or higher)
echo "Installing CMake..."
sudo apt-get install -y software-properties-common
sudo apt-add-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install -y cmake

# Install Boost
echo "Installing Boost..."
sudo apt-get install -y libboost-all-dev

# Install BZIP2
echo "Installing BZIP2..."
sudo apt-get install -y libbz2-dev

# Install OpenSSL
echo "Installing OpenSSL..."
sudo apt-get install -y libssl-dev

# Install libcurl
echo "Installing libcurl..."
sudo apt-get install -y libcurl4-openssl-dev

# Install g++
echo "Installing g++..."
sudo apt-get install -y g++

# Install Python dependencies directly
echo "Installing Python dependencies in the system-wide Python environment..."
sudo apt-get install -y python3-pip
pip3 install --upgrade pip --break-system-packages
pip3 install pandas seaborn --break-system-packages

# Install Java version 8
echo "Installing Java 8..."
sudo apt-get install -y openjdk-8-jdk

# Set Java 8 as default
echo "Setting Java 8 as the default Java version..."
sudo update-alternatives --config java <<<'1'

# Verify Java installation
java -version

# Install Maven
echo "Installing Maven..."
sudo apt-get install -y maven

# Verify Maven installation
mvn -v

# Final check
echo "All dependencies installed successfully: Clang++, CMake, Boost, BZIP2, OpenSSL, libcurl, g++, Python (pip, pandas, seaborn), Java 8, and Maven."
