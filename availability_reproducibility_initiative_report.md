# Availability and Reproducibility Initiative Report

---

1) **Source Code**
   - [Source code](/publication/source_code)

2) **Software/Library Dependencies**
   - __Clang++__
   - __CMake__ 3.20 or higher

3) **Hardware Requirements**
   - [Intel Ice Lake](https://en.wikipedia.org/wiki/Ice_Lake_(microprocessor))
   - [Zen 3](https://en.wikipedia.org/wiki/Zen_3)
   - [Apple M1](https://en.wikipedia.org/wiki/Apple_M1)
   - [AWS Graviton 2](https://en.wikipedia.org/wiki/AWS_Graviton)
   - [AWS Graviton 3](https://en.wikipedia.org/wiki/AWS_Graviton)

4) **Data Collection and Plotting Scripts**
   - [Detailed, fully automated scripts](/publication/plotter) that collect data and plot all figures in the paper.
      - *Figure 6*: Script missing and needs to be completed.
   - Figures generated are a combination of data already in the repository and additional data produced by running the script.

5) **Tables from Experimental Results**
   - Script should also generate tables from experimental results in a Markdown.

6) **Documentation**
   - Compilation, deployment, execution, and script usage instructions are in [BENCHMARKING.md](/BENCHMARKING.md), detailing how to replicate experiments and benchmarks presented in our [publication](https://dl.acm.org/doi/pdf/10.1145/3626717).

7) **Master Script**
   - A link to [a single master script](/publication/master_script/master_script.sh) that runs the experiments, collects data, and manages workflow.
   - *Current Configuration Notes*:
      - On MacBooks, CPU details require `sudo` to access the cycle counter.
      - Font warning messages do not impact script functionality but may require additional font installation.

8) **Deployment and Execution Estimates**
   - Estimated deployment time and effort: X hours
   - Execution time for the full workflow (for reviewer preparation): 1 hour

---

Based on [SIGMOD ARΙ Package Requirements and Guidelines](https://docs.google.com/document/d/1_pheZ2p9Nc8qhtcOpNINm7AxFpPpkpC1n60jJdyr-uk/export?format=pdf&attachment=false)
