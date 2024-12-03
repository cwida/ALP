# Elf: Erasing-based Lossless Floating-Point Compression

***
elf is an erasure-based floating-point data compression algorithm with a high compression ratio.

Developers can follow the steps below for compression testing.

## Elf feature

- Elf can greatly increase the number of trailing zeros in XORed results, which enhances the compression ratio with a
  theoretical guarantee
- Elf algorithm takes only O (1) in both time complexity and space complexity.
- ELf adopt unique coding method for the XORed results with many trailing zeros.
- The erase operation in this project is used as a preprocessing step for all XOR-based compression algorithms.

## Project Structure

This project mainly includes the following various compression algorithms:

- The main code for the ***Elf*** algorithm is in the *org/urbcomp/startdb/compress/elf* package.

- The main code for the ***Chimp*** algorithm is in the *gr/aueb/delorean/chimp* package.

- The main code for the ***Gorilla*** algorithm is in the *fi/iki/yak/ts/compression/gorilla* package.

- The main code for the ***FPC*** algorithm is in the *com/github/kutschkem/fpc* package.

- The main code for other general compression algorithms is in the *org/apache/hadoop/hbase/io/compress* package.

### ELF Structure

ELF includes *compressor* and *decompressor* packages as well as *xorcompressor* and *xordecompressor* based on erasure
design.

#### compressor package

This package includes 7 different XOR-based compression algorithms and gives a standard **ICompressor** interface. The
erase operation is abstracted as an **AbstractElfCompressor**.
- ElfCompressor: This class is the complete elf compression algorithm.
- ElfOnChimpCompressor: This class is pre-processed for erasure and then compressed using the Chimp algorithm.
- ElfOnChimpNCompressor: This class is pre-processed for erasure and then compressed using the Chimp128 algorithm.
- ElfOnGorillaCompressorOS: This class is pre-processed for erasure and then compressed using the Gorilla algorithm.
- GorillaCompressorOS: This class is the Gorilla algorithm using Bitstream I/O optimization.
- ChimpCompressor: This class is the original chimp algorithm.
- ChimpNCompressor: This class is the original chimp128 algorithm.

#### decompressor package

This package includes the decompressors corresponding to the above 7 compressors and gives the standard **IDecompressor** interface

#### xorcompressor package

This package is a compressed encoding of post-erase data designed for XOR-based operations

#### dexorcompressor package

This package is a decompression of the erased data designed based on the XOR-based operation code.

## TEST ELF

We recommend IntelliJ IDEA for developing projects.

### Prerequisites for testing

The following resources need to be downloaded and installed:

- Java 8 download: https://www.oracle.com/java/technologies/downloads/#java8
- IntelliJ IDEA download: https://www.jetbrains.com/idea/
- git download:https://git-scm.com/download

Download and install jdk-8, IntelliJ IDEA and git.

### Clone code

1. Open *IntelliJ IDEA*, find the *git* column, and select *Clone...*

2. In the *Repository URL* interface, *Version control* selects *git*

3. URL filling: *https://github.com/Spatio-Temporal-Lab/elf.git*

### Set JDK

File -> Project Structure -> Project -> Project SDK -> *add SDK*

Click *JDK* to select the address where you want to download jdk-8

### Test ELF

Select the *org/urbcomp/startdb/compress/elf* package in the *test* folder, which includes tests for 64bits Double data
and 32bits Float data

#### Double data test:

In *doubleprecision* package

- The **TestCompressor** class includes compression tests for 22 data sets. The test results are saved in *result/result.csv* in resource.
- The **TestBeta** class is a compression test for different beta of data. Two data sets with long mantissa are selected
  and different bits are reserved for compression test. The test results are saved in *result/resultBeta.csv* in
  resource.

#### Float data test:

In *singleprecision* package

- The **TestCompressor** class includes compression tests for 22 data sets. The test results are saved in *
  result32/result.csv* in resource.

