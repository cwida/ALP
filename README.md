# ALP: Adaptive Lossless Floating-Point Compression

Lossless floating-point compression algorithm for `double`/`float` data type. ALP significantly improves over all
previous floating-point encodings in both speed and compression ratio (figure below; each dot represents a dataset).

<p align="center">
        <img src="/publication/alp_results.png" alt="ALP Benchmarks" height="350">
</p>

- ⚡ **High Speed**: Scans 44x faster than Gorilla, 64x faster than Chimp, 31x faster than Zstd. Compresses 11x faster
  than Zstd, 138x faster than PDE and x10 faster than Chimp.
- ✅ **High Compression**: 50% more compression than Gorillas. 24% more than Chimp128. On par with Zstd level 3.
- ✅ **Adaps to data**: By using a two-stage algorithm that first samples row-groups and then vectors.
- ✅ **Scalar code**: Auto-vectorizes thanks to [FastLanes](https://github.com/cwida/FastLanes).
- ✅ **Lightweight Encoding**: Compression and decompression occurs in blocks of 1024 values. Ideal for columnar
  databases.
- ✅ **Proven Effectiveness**: Effectiveness and speed led to deprecating Chimp128 and Patas in DuckDB.
- ✅ **Works on difficult floats**: Can losslessly compress even floats present as ML models parameters better than Zstd
  and all other encodings.

To *rigorously* benchmark ALP with your own data we provide our [ALP primitives](#alp-primitives) as a single C++ header
file.

ALP details can be found in the [publication](https://dl.acm.org/doi/pdf/10.1145/3626717).

## Availability & Reproducibility Initiative (ARI) Report

In [this report](availability_reproducibility_initiative_report.md), we explain how to replicate the experiments and
benchmarks according to the format requested
in [SIGMOD ARΙ Package Requirements and Guidelines](https://reproducibility.sigmod.org/2024/).

On the benchmarked datasets from our publication:

- ALP achieves on average **x3 compression ratios** (sometimes much, much higher).
- ALP encodes on average 0.5 doubles per CPU cycle.
- ALP decodes on average 2.6 doubles per CPU cycle.

### Used By

<table>
  <tr>
    <td>
      <p align="left">
        <img src="https://raw.githubusercontent.com/duckdb/duckdb/main/logo/DuckDB_Logo-horizontal.png" alt="DuckDB" height="50">
      </p>
    </td>
    <td>
      <p align="left">
        <a href="https://github.com/cwida/FastLanes">FastLanes</a>
      </p>
    </td>
  </tr>
</table>

### Contents

- [ALP in a Nutshell](#alp-in-a-nutshell)
- [Quickstart](#quickstart)
- [Building and Running](#building-and-running)
- [ALP Primitives](#alp-primitives)
- [ALP in DuckDB](#alp-in-duckdb)
- [Benchmarking (Replicating Paper Experiments)](#benchmarking-replicating-paper-experiments)

## ALP in a Nutshell

ALP has two compression schemes: `ALP` for doubles/floats which were once decimals, and `ALP_RD` for true
double/floats (e.g. the ones which stem from many calculations, scientific data, ML weights).

`ALP` losslessly transforms doubles/floats to integer values with two multiplications to FOR+BitPack them into only the
necessary bits. This is a strongly enhanced version of [PseudoDecimals](https://dl.acm.org/doi/abs/10.1145/3589263).

`ALP_RD` splits the doubles/floats bitwise representations into two parts (left and right). The left part is encoded
with a Dictionary compression and the right part is Bitpacked to just the necessary bits.

Both encodings operate in vectors of 1024 values at a time (fit *vectorized execution*) and leverage in-vector
commonalities to achieve higher compression ratios and be faster (by avoiding per-value adaptivity) than other methods.

Both encodings encode outliers as *exceptions* to achieve higher compression ratios.

## Building and Running

Requirements:

1) __Clang++__
2) __CMake__ 3.20 or higher

## ALP Primitives

You can make your own [de]compression API by using ALP primitives. An example of the usage of these can be found in our
simple [compression](/include/alp/compressor.hpp) and [decompression](/include/alp/decompressor.hpp) API. The decoding
primitives of ALP are auto-vectorized thanks to [FastLanes](https://github.com/cwida/FastLanes). For **benchmarking**
purposes, we recommend you use these primitives.

You can use these by including our library in your code: `#include "alp.hpp"`.

Check the full documentation of these on the [PRIMITIVES.MD](/PRIMITIVES.md) readme.

## ALP in DuckDB

ALP replaced Chimp128 and Patas in [DuckDB](https://github.com/duckdb/duckdb/pull/9635). In DuckDB, ALP is **x2-4 times
faster** than Patas (at decompression) achieving **twice as high compression ratios** (sometimes even much more). DuckDB
can be used to quickly test ALP on custom data, however, we advise against doing so if your purpose is to rigorously
benchmark ALP against other algorithms.

[Here](https://github.com/duckdb/duckdb/blob/main/benchmark/micro/compression/alp/alp_read.benchmark) you can find a
basic example on how to load data in DuckDB forcing ALP to be used as compression method. These statements can be called
using the Python API.

**Please note**: ALP inside DuckDB: i) Is slower than using our primitives presented here, and ii) compression ratios
can be slightly worse due to the metadata needed to skip vectors and DuckDB storage layout.

## FCBench

We have benchmarked ALP compression ratios on the datasets presented
on [FCBench](https://www.vldb.org/pvldb/vol17/p1418-tao.pdf). ALP comes on top with an average **compression ratio of
2.08** compared to the best compressor in the benchmark (Bitshuffle + Zstd with 1.47). ALP is superior even despite the
benchmark doing horizontal compression instead of columnar compression (i.e. values from multiple columns in a table are
compressed together).

