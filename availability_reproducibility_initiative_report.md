# Availability and Reproducibility Initiative Report

---

## 1. Source Code
- [Source code](/publication/source_code)

## 2. Software/Library Dependencies
- **Clang++**
- **CMake** (version 3.22 or higher)
- **Boost** (required for end-to-end benchmarks)
- **BZIP2** (required for PED benchmarks)
- **OpenSSL** (required for PED benchmarks)
- **libcurl** (required for PED benchmarks)
- **g++** (required for PED benchmarks)
- **pandas** (required for plots)
- **seaborn** (required for plots)
- **Java 8** (required for ELF)
- **[Maven 3.9.9](https://maven.apache.org/download.cgi)** (required for ELF)

**This [script](/publication/script/install_dependencies.sh) can be used to install all necessary dependencies on Ubuntu.**

## 3. Hardware Requirements
- **[Intel Ice Lake](https://en.wikipedia.org/wiki/Ice_Lake_(microprocessor))** (x86_64, AVX512): Recommended AWS instance type: [i4i.4xlarge](https://aws.amazon.com/ec2/instance-types/i4i/), with at least 16 cores.
- **[AMD Zen 3](https://en.wikipedia.org/wiki/Zen_3)** (x86_64, AVX2): Recommended AWS instance types: M6a and C6a.
- **[Apple M1](https://en.wikipedia.org/wiki/Apple_M1)**
- **[AWS Graviton 2](https://en.wikipedia.org/wiki/AWS_Graviton)** (ARM64, NEON): Recommended AWS instance types: M6g, C6g, R6g, and T4g.
- **[AWS Graviton 3](https://en.wikipedia.org/wiki/AWS_Graviton)** (ARM64, NEON): Recommended AWS instance types: M7g, C7g, and R7g.

> For further questions about hardware recommendations, feel free to ask.

## 4. Data Collection and Plotting Scripts
- **[Plotter](/publication/plotter)**: Collects data and generates all figures in the paper.
  - Uses data from the repository and additional data produced by running the script.
  - **Note**: The script for *Figure 6* is missing and needs to be completed.
- **[Table Generator](/publication/script/generate_tables.py)**: Generates Tables 4 and 7 from the paper, stores the generated tables in Markdown format in [result_dir](/publication/tables/), and prints the tables to the terminal.

## 5. Tables from Experimental Results
- The master script generates experimental result tables in Markdown format.

## 6. Documentation
- Detailed instructions for compilation, deployment, execution, and script usage are provided in **[BENCHMARKING.md](/BENCHMARKING.md)**.
- Guidelines explain how to replicate experiments and benchmarks from the **[publication](https://dl.acm.org/doi/pdf/10.1145/3626717)**.

## 7. Master Script
- **[Master script](/publication/script/master_script.sh)**: Automates experiments, data collection, and workflow management.
  - **Outputs**:
    - A log file (`master_script.log`) with step completion reports (green) and tables (brown).
  - **MacBook Note**: Requires `sudo` access to enable the cycle counter.
  - **Font Warnings**: May require additional font installation, which does not impact functionality.

## 8. Deployment and Execution Estimates
- **Deployment time and effort**: X hours
- **Execution time**: Approximately 1 hour for the full workflow (for reviewer preparation).

---

**Based on**: [SIGMOD ARI Package Requirements and Guidelines](https://docs.google.com/document/d/1_pheZ2p9Nc8qhtcOpNINm7AxFpPpkpC1n60jJdyr-uk/export?format=pdf&attachment=false)
