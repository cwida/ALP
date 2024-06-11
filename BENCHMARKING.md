# Benchmarking

Here we explain how to replicate the experiments presented in our [publication](https://dl.acm.org/doi/pdf/10.1145/3626717) and how to benchmark with your own data. 

On the benchmarked datasets from our publication:
- ALP achieves x3 compression ratios on average (sometimes much much higher).
- ALP encodes 0.5 values per CPU cycle.
- ALP decodes 2.6 values per CPU cycle.

On [FCBench](https://www.vldb.org/pvldb/vol17/p1418-tao.pdf):
- ALP achieves a compression ratio of 2.08 (beating all other compressors)

## Contents
- [Contents](#contents)
- [Build](#build)
- [Downloading Data](#downloading-data)
  - [Environment Variables](#environment-variables)
  - [Setup Data](#setup-data)
- [Compression Ratios Experiment](#compression-ratios-experiment)
  - [ALP](#alp)
  - [Chimp, Chimp128, Gorillas, Patas, Zstd Compression Ratios](#chimp-chimp128-gorillas-patas-zstd-compression-ratios)
- [Speed Tests](#speed-tests)
  - [ALP Encoding Speed Test](#alp-encoding-speed-test)
  - [ALP Decoding Speed Test](#alp-decoding-speed-test)
  - [ALP RD Encoding Speed Test](#alp-rd-encoding-speed-test)
  - [ALP RD Decoding Speed Test](#alp-rd-decoding-speed-test)
  - [Chimp, Chimp128, Gorillas, Patas, Zstd Speed Test](#chimp-chimp128-gorillas-patas-zstd-speed-test)
  - [PseudoDecimals Speed Test](#pseudodecimals-speed-test)
  - [ELF Speed Test](#elf-speed-test)
- [FCBench](#fcbench)


## Build

```shell
cmake [OPTIONS] .
make
```

Options: 
- `-DALP_BUILD_EXAMPLE=ON`: Build the examples in `/example`
- `-DALP_BUILD_TESTING=ON`: Build ALP correctness tests in `/test`
- `-DALP_BUILD_BENCHMARKING=ON`: Build speed and compression ratio benchmarks in `/benchmarks`
- `-DALP_BUILD_GENERATED=ON`: Build FastLanes generated code in `/generated`

You can also set these options directly inside the `CMakeLists.txt`

## Downloading Data

You can download the datasets shown in our publication [here](https://drive.google.com/drive/folders/167faTwZJjqJMKM9Yc6E7KF5LUbsitxJS?usp=sharing) (`complete_binaries.zip`). They are in a binary format (64 bit doubles one after another). These are the files we used to benchmark ALP compression ratios in the [publication](https://dl.acm.org/doi/pdf/10.1145/3626717).

In addition to this, inside `data/datasets_transformer.ipynb` you can find a [Jupyter Notebook script](/data/datasets_transformer.ipynb) with guidelines to download the datasets from their original source and code to transform them to a binary format (64 bits doubles). Note that some of these require a heavy pre-processing phase.

### Environment Variables
Set the environment variable `ALP_DATASET_DIR_PATH` with the path to the directory in which the complete binary datasets are located; either on your env or manually on the [column.hpp](/data/include/column.hpp) file. 

### Setup Data
Inside `data/include/double_columns.hpp` you can find an array containing information regarding the datasets used to benchmark ALP. Datasets information includes a path to a sample of one vector (1024 values) in CSV format (inside `/data/samples/`) and a path to the entire file in binary format. 

The binary file is used to benchmark ALP compression ratios, while the CSV sample is used to benchmark ALP speed. To ensure the correctness of the speed tests we also keep extra variables from each dataset, which include the number of exceptions and the bitwidth resulting after compression (unless the algorithm changes, these should remain consistent), and the factor/exponent indexes used to encode/decode the doubles into integers. 

To set up the data you want to run the test on, add or remove entries in the array found in [double_columns.hpp](/data/include/double_columns.hpp) and `make` again. The data needed for each entry is detailed in [column.hpp](/data/include/column.hpp). To replicate the compression ratio tests you only need to set the dataset id, name, and binary_file_path. 

## Compression Ratios Experiment

### ALP
After building and setting up the data, and the `ALP_DATASET_DIR_PATH` env variable, run the following:
```sh
./benchmarks/bench_compression_ratio/bench_alp_compression_ratio
```
This will execute the tests found in the [/benchmarks/bench_compression_ratio/alp.cpp](/benchmarks/bench_compression_ratio/alp.cpp) file, which will compress an entire binary file and write the resulting (estimated) compression ratio results (in bits/value) from the datasets in [double_columns.hpp](/data/include/double_columns.hpp), on the `publication` directory. One CSV file will be created for the datasets which use the `ALP` scheme and another one for the ones which use the `ALP_RD` scheme. Note that this is a dry compression (compressed data is not stored).

### Chimp, Chimp128, Gorillas, Patas, Zstd Compression Ratios
After building and setting up the data, and the `ALP_DATASET_DIR_PATH` env variable, run the following: `./benchmarks/bench_compression_ratio/bench_{algorithm}_compression_ratio`, in which `algorithm` can be: `chimp|chimp128|gorillas|patas|zstd`. One CSV file will be created for each encoding and for each dataset on the `publication` directory. Note that this is a dry compression (compressed data is not stored). For PDE and ELF, we used their own code for compression ratios.

## Speed Tests

All of these tests read the CSV samples files locations from the dataset array. Therefore, to test with your own data, add your dataset to this array. Note that these experiments are performed on 1024 values. Why? Check Section 4 of the [publication](https://dl.acm.org/doi/pdf/10.1145/3626717).

### ALP Encoding Speed Test
Encoding is comprised of the `encode`, `analyze_ffor`, and `ffor` primitives. Benchmarked by running: `./benchmarks/bench_speed/bench_alp_encode`. Results are located on `publication/results/`.

### ALP Decoding Speed Test
Fused decoding is comprised of the `falp` and the `patch_exceptions` primitives. Unfused decoding is comprised of the `unffor`, `decode` and `patch_exceptions` primitives. Benchmark both fused and unfused at the same time on different implementations and Architectures/ISAs by running the commands below. Results are located on `publication/results/`.

| Implementation  | Command                                                                                                    |
|-----------------|------------------------------------------------------------------------------------------------------------|
| Scalar          | `./generated/fallback/scalar_nav_uf1/fallback_scalar_nav_1024_uf1_falp_bench`                          |
| SIMD            | `./generated/{Arch}/{Arch}_{extension}_intrinsic_uf1/{Arch}_{extension}_intrinsic_1024_uf1_falp_bench` |
| Auto-Vectorized | `./generated/fallback/scalar_aav_uf1/fallback_scalar_aav_1024_uf1_falp_bench`                          |

While the *correctness* can be tested by running:

| Implementation  | Command                                                                                                   |
|-----------------|-----------------------------------------------------------------------------------------------------------|
| Scalar          | `./generated/fallback/scalar_nav_uf1/fallback_scalar_nav_1024_uf1_falp_test`                          |
| SIMD            | `./generated/{Arch}/{Arch}_{extension}_intrinsic_uf1/{Arch}_{extension}_intrinsic_1024_uf1_falp_test` |
| Auto-Vectorized | `./generated/fallback/scalar_aav_uf1/fallback_scalar_aav_1024_uf1_falp_test`                          |

The source file of the `falp` primitive (FUSED ALP+FOR+Bitpack generated by [FastLanes](https://github.com/cwida/FastLanes)) for each different implementation are at:

| Implementation  | Source File                                                                                                |
|-----------------|------------------------------------------------------------------------------------------------------------|
| Scalar          | `generated/fallback/scalar_nav_uf1/fallback_scalar_nav_1024_uf1_falp_src.cpp`                          |
| SIMD            | `generated/{Arch}/{Arch}_{extension}_intrinsic_uf1/{Arch}_{extension}_intrinsic_1024_uf1_falp_src.cpp` |
| Auto-Vectorized | `generated/fallback/scalar_aav_uf1/fallback_scalar_aav_1024_uf1_falp_src.cpp`                          |


Architectures and ISAs:

| Architecture {Arch} | ISA {extension}    |
|--------------|---------|
| arm64v8      | neon    |
| arm64v8      | sve     |
| wasm         | simd128 |
| x86_64       | sse     |
| x86_64       | avx2    |
| x86_64       | avx512bw|

### ALP RD Encoding Speed Test
Encoding is comprised of `rd_encode` and two calls to `ffor` (for both the left and right parts). Benchmarked by running: `./benchmarks/bench_speed/bench_alp_cutter_encode`. Results are located on `publication/results/`.

### ALP RD Decoding Speed Test
Decoding is comprised of two calls to `unffor` (for both the left and right parts) and the `rd_decode` primitives. Benchmarked by running: `./benchmarks/bench_speed/bench_alp_cutter_decode`. Results are located on `publication/results/`.

### Chimp, Chimp128, Gorillas, Patas, Zstd Speed Test
Benchmarked both decoding and encoding by running `./benchmarks/bench_speed/bench_{algorithm}`, in which `algorithm` can be: `chimp|chimp128|gorillas|patas|zstd`. Results are located on `publication/results/i4i`.

### PseudoDecimals Speed Test
We benchmarked PseudoDecimals within BtrBlocks. Results are located on `publication/results/i4i`.

### ELF Speed Test
We benchmarked Elf using their Java implementation.

## FCBench
We have benchmarked ALP compression ratios on the datasets presented on [FCBench](https://www.vldb.org/pvldb/vol17/p1418-tao.pdf). ALP comes on top with an average **compression ratio of 2.08** compared to the best compressor in the benchmark (Bitshuffle + Zstd with 1.47). ALP is superior even despite the benchmark doing horizontal compression instead of columnar compression (i.e. values from multiple columns in a table are compressed together). 