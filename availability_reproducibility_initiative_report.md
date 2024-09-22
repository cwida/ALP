# Availability and reproducibility initiative report

---

1) [source_code](/publication/source_code)

2) Description of software/library dependencies:
    1) __Clang++__
    2) __CMake__ 3.20 or higher
3) Description of hardware needed
    1) [Intel Ice Lake](https://en.wikipedia.org/wiki/Ice_Lake_(microprocessor))
    2) [Zen 3](https://en.wikipedia.org/wiki/Zen_3)
    3) [Apple M1](https://en.wikipedia.org/wiki/Apple_M1)
    4) [AWS Graviton 2](https://en.wikipedia.org/wiki/AWS_Graviton)
    5) [AWS Graviton 3](https://en.wikipedia.org/wiki/AWS_Graviton)
4) [Link](/publication/plotter) to detailed fully automated scripts that collect data and plot all figures of the
   papers
    - figure 6 : todo

5) Documentation on how to compile, deploy, run the code, and use the scripts:
    - Follow the [Publication CI](.github/workflows/PUBLICATION.yaml)
    - In [BENCHMARKING.md](/BENCHMARKING.md) we detail how to replicate the experiments and benchmarks presented in
      our [publication](https://dl.acm.org/doi/pdf/10.1145/3626717).
6) A link to a single master script that runs the experiments, collects the data, and
    - follow the [Publication CI](.github/workflows/PUBLICATION.yaml)
7)
    - An estimation of the deployment time and effort : X hours
    - The execution time of the whole workflow to prepare the reviewers : Y hours

Based on [SIGMOD ARΙ Package Requirements and
Guidelines](https://docs.google.com/document/d/1_pheZ2p9Nc8qhtcOpNINm7AxFpPpkpC1n60jJdyr-uk/export?format=pdf&attachment=false)
