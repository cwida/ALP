# Benchmarking

This guide explains how to:
- Replicate the experiments presented in our [publication](https://dl.acm.org/doi/pdf/10.1145/3626717)
- Benchmark with your own data

## How to Replicate the Experiments

### 1. Download Data

You can download the datasets used in our publication [here](https://drive.google.com/drive/folders/167faTwZJjqJMKM9Yc6E7KF5LUbsitxJS?usp=sharing) (`complete_binaries.zip`). These files are in binary format (64-bit doubles) and were used thourght the benchmarks in our paper.

### 2. Set Environment Variables

Set the environment variable `ALP_DATASET_DIR_PATH` with the path to the directory containing the binary datasets that you downloaded in step 1.

Additionally, inside `data/datasets_transformer.ipynb`, you can find a [Jupyter Notebook script](/data/datasets_transformer.ipynb) with guidelines to download datasets from their original source and code to convert them to binary format (64-bit doubles). Some datasets may require heavy pre-processing.

### 3. Set the Correct Toolchain File

There are two toolchain files in the [toolchain directory](toolchain). Adjust them based on the `clang` and `clang++` versions you are using.

### 4. Run the Benchmark Script

Run the [master_script.sh](publication/script/master_script.sh) on the following architectures:

- **Intel Ice Lake (x86_64, AVX512):** M6i and C6i instances
- **AMD Zen3 (x86_64, AVX2):** M6a and C6a instances
- **AWS Graviton2 (ARM64, NEON):** M6g, C6g, R6g, and T4g instances
- **AWS Graviton3 (ARM64, NEON):** M7g, C7g, and R7g instances
- **Apple M1:** Note that on M1, you should run the following script with `sudo` permissions.


```shell
./publication/master_script/master_script.sh
```

### 5. Details:

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

## Compression Ratios Experiment

### ALP


```sh
./publication/source_code/bench_compression_ratio/publication_bench_alp_compression_ratio
```

This target will compress an entire binary file and write the
resulting (estimated) compression ratio results (in bits/value)
from the datasets in `double_columns.hpp` to the [publication directory](publication/results).
One CSV file will be created for the datasets which use the ALP scheme
and another one for those which use the ALP_RD scheme. Note that this
is a dry compression (compressed data is not stored).

### Chimp, Chimp128, Gorillas, Patas, Zstd Compression Ratios

The following target
`./publication/source_code/bench_compression_ratio/bench_{algorithm}_compression_ratio`, in which `algorithm` can be:
`chimp|chimp128|gorillas|patas|zstd` will create a csv file for each encoding and for each dataset on the
`publication` directory. Note that this is a dry compression (compressed data is not stored). For PDE and ELF, we used
their own code for compression ratios.

## Speed Tests

All of these tests read the CSV samples files locations from the dataset array. Therefore, to test with your own data,
add your dataset to this array. Note that these experiments are performed on 1024 values. Why? Check Section 4 of
the [publication](https://dl.acm.org/doi/pdf/10.1145/3626717).

### ALP Encoding Speed Test

Encoding is comprised of the `encode`, `analyze_ffor`, and `ffor` primitives. Benchmarked by running:
`./publication/source_code/bench_speed/bench_alp_encode`. Results are located on `publication/results/`.

### ALP Decoding Speed Test

Fused decoding is comprised of the `falp` and the `patch_exceptions` primitives. Unfused decoding is comprised of the
`unffor`, `decode` and `patch_exceptions` primitives. Benchmark both fused and unfused at the same time on different
implementations and Architectures/ISAs by running the commands below. Results are located on `publication/results/`.

| Implementation  | Command                                                                                                |
|-----------------|--------------------------------------------------------------------------------------------------------|
| Scalar          | `./publication/source_code/generated/fallback/scalar_nav_uf1/fallback_scalar_nav_1024_uf1_falp_bench`                          |
| SIMD            | `./publication/source_code/generated/{Arch}/{Arch}_{extension}_intrinsic_uf1/{Arch}_{extension}_intrinsic_1024_uf1_falp_bench` |
| Auto-Vectorized | `./publication/source_code/generated/fallback/scalar_aav_uf1/fallback_scalar_aav_1024_uf1_falp_bench`                          |

While the *correctness* can be tested by running:

| Implementation  | Command                                                                                               |
|-----------------|-------------------------------------------------------------------------------------------------------|
| Scalar          | `/publication/source_code/generated/fallback/scalar_nav_uf1/fallback_scalar_nav_1024_uf1_falp_test`                          |
| SIMD            | `/publication/source_code/generated/{Arch}/{Arch}_{extension}_intrinsic_uf1/{Arch}_{extension}_intrinsic_1024_uf1_falp_test` |
| Auto-Vectorized | `/publication/source_code/generated/fallback/scalar_aav_uf1/fallback_scalar_aav_1024_uf1_falp_test`                          |

The source file of the `falp` primitive (FUSED ALP+FOR+Bitpack generated
by [FastLanes](https://github.com/cwida/FastLanes)) for each different implementation are at:

| Implementation  | Source File                                                                                            |
|-----------------|--------------------------------------------------------------------------------------------------------|
| Scalar          | `/publication/source_code/generated/fallback/scalar_nav_uf1/fallback_scalar_nav_1024_uf1_falp_src.cpp`                          |
| SIMD            | `/publication/source_code/generated/{Arch}/{Arch}_{extension}_intrinsic_uf1/{Arch}_{extension}_intrinsic_1024_uf1_falp_src.cpp` |
| Auto-Vectorized | `/publication/source_code/generated/fallback/scalar_aav_uf1/fallback_scalar_aav_1024_uf1_falp_src.cpp`                          |

Architectures and ISAs:

| Architecture {Arch} | ISA {extension} |
|---------------------|-----------------|
| arm64v8             | neon            |
| arm64v8             | sve             |
| wasm                | simd128         |
| x86_64              | sse             |
| x86_64              | avx2            |
| x86_64              | avx512bw        |

### ALP RD Encoding Speed Test

Encoding is comprised of `rd_encode` and two calls to `ffor` (for both the left and right parts). Benchmarked by
running: `./publication/source_code/bench_speed/bench_alp_cutter_encode`. Results are located on `publication/results/`.

### ALP RD Decoding Speed Test

Decoding is comprised of two calls to `unffor` (for both the left and right parts) and the `rd_decode` primitives.
Benchmarked by running: `./publication/source_code/bench_speed/bench_alp_cutter_decode`. Results are located on
`publication/results/`.

### Chimp, Chimp128, Gorillas, Patas, Zstd Speed Test

Benchmarked both decoding and encoding by running `./publication/source_code/bench_speed/bench_{algorithm}`, in which
`algorithm` can
be: `chimp|chimp128|gorillas|patas|zstd`. Results are located on `publication/results/i4i`.

### PseudoDecimals Speed Test

We benchmarked PseudoDecimals within BtrBlocks. Results are located on `publication/results/i4i`.

### ELF Speed Test

We benchmarked Elf using their Java implementation.