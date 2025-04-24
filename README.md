# ALP: Adaptive Lossless Floating-Point Compression

**Authors**: Azim Afroozeh, Leonardo Kuffó, Peter Boncz  
**Conference**: ACM SIGMOD 2024  

---

## 📄 What is this repo?

This repository contains the source code and benchmarks for the paper [_ALP: Adaptive Lossless Floating-Point Compression_](https://dl.acm.org/doi/abs/10.1145/3626717), published at ACM SIGMOD 2024.

**ALP** is a state-of-the-art lossless compression algorithm tailored for IEEE 754 floating-point data. It adaptively selects between two schemes:

- **ALP**: Converts floats/doubles that originate from decimal values into integers using enhanced PseudoDecimals, then applies Frame-of-Reference (FOR) and Bit-Packing.
- **ALP_RD**: For general floats/doubles, it splits the bitwise representation into left and right parts, compressing them using Dictionary encoding and Bit-Packing, respectively.

This adaptive approach enables ALP to outperform existing methods like Gorilla, Chimp128, and Zstd in both compression efficiency and speed.

---

## 📊 Results Highlights

![ALP Results](alp_results.png)

This is the highlight of how ALP performs, These results shows ALP’s superiority across all three key metrics of a compression algorithm:  
**Decoding Speed**, **Compression Ratio**, and **Compression Speed** above other schems.

---

## 🚀 How to Reproduce Results

```bash
./publication/script/master_script.sh
```

For more information about reproducing our benchmarks, read our guide [here](availability_reproducibility_initiative_report.md),  
or see the official ACM reproducibility report:  
[https://dl.acm.org/doi/10.1145/3687998.3717057](https://dl.acm.org/doi/10.1145/3687998.3717057)

---

## 📦 Want to Benchmark Your Dataset?

Check out our guide: [How to Benchmark Your Dataset](how_to_benchmark_your_dataset.md)  
It explains how to benchmarko ALP on your dataset.

---

## 🧰 Repository Structure

- `src/`: Core implementation of ALP and ALP_RD
- `benchmarks/`: Benchmarking tools and datasets
- `include/`: Header files for integration
- `scripts/`: Utility scripts for data processing
- `test/`: Unit tests for validation
- `publication/`: Related publications and supplementary materials

---

## 📚 Publications

- **Conference Paper**:  
  _ALP: Adaptive Lossless Floating-Point Compression_, ACM SIGMOD 2024  
  [https://dl.acm.org/doi/10.1145/3626717](https://dl.acm.org/doi/10.1145/3626717)

- **Reproducibility Report**:  
  _Reproducibility Report for ACM SIGMOD 2024 Paper: 'ALP: Adaptive Lossless Floating-Point Compression'_  
  [https://dl.acm.org/doi/10.1145/3687998.3717057](https://dl.acm.org/doi/10.1145/3687998.3717057)

---

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## 📬 Contact

For questions, announcements, or collaborations, please join our Discord channel:  
[https://discord.gg/2ngmRaRW](https://discord.gg/2ngmRaRW)
```


---

## 🧩 Used By

ALP is integrated into the following systems:

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/duckdb/duckdb/pull/8217">
        <img src="https://upload.wikimedia.org/wikipedia/commons/5/5c/DuckDB_Logo.png" width="80"/><br/>
        <strong>DuckDB</strong>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/cwida/FastLanes">
        <img src="https://raw.githubusercontent.com/cwida/FastLanes/main/assets/logo.svg" width="80"/><br/>
        <strong>FastLanes</strong>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/kuzudb/kuzu/pull/XXX">
        <img src="https://raw.githubusercontent.com/kuzudb/kuzu/main/docs/logo.svg" width="80"/><br/>
        <strong>KuzuDB</strong>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/ClickHouse/ClickHouse/pull/XXXX">
        <img src="https://avatars.githubusercontent.com/u/30824861?s=200&v=4" width="80"/><br/>
        <strong>ClickHouse</strong>
      </a>
    </td>
  </tr>
</table>
