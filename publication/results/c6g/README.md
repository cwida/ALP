# Info 
[c6g](https://aws.amazon.com/ec2/instance-types/c6g/):

---

- `lscpu`
  - ```shell
    
    ubuntu@ip-172-31-75-129:~$ lscpu
    Architecture:           aarch64
    CPU op-mode(s):       32-bit, 64-bit
    Byte Order:           Little Endian
    CPU(s):                 1
    On-line CPU(s) list:  0
    Vendor ID:              ARM
    Model name:           Neoverse-N1
    Model:              1
    Thread(s) per core: 1
    Core(s) per socket: 1
    Socket(s):          1
    Stepping:           r3p1
    BogoMIPS:           243.75
    Flags:              fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm lrcpc dcpop asimddp ssbs
    Caches (sum of all):    
    L1d:                  64 KiB (1 instance)
    L1i:                  64 KiB (1 instance)
    L2:                   1 MiB (1 instance)
    L3:                   32 MiB (1 instance)
    NUMA:                   
    NUMA node(s):         1
    NUMA node0 CPU(s):    0
    Vulnerabilities:        
    Itlb multihit:        Not affected
    L1tf:                 Not affected
    Mds:                  Not affected
    Meltdown:             Not affected
    Mmio stale data:      Not affected
    Retbleed:             Not affected
    Spec store bypass:    Mitigation; Speculative Store Bypass disabled via prctl
    Spectre v1:           Mitigation; __user pointer sanitization
    Spectre v2:           Mitigation; CSV2, BHB
    Srbds:                Not affected
    Tsx async abort:      Not affected
    ubuntu@ip-172-31-75-129:~$

    ```
---
## History

---