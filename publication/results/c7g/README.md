# Info 
[c7g](https://aws.amazon.com/ec2/instance-types/cgg/):

---

- `lscpu`
    - ```shell
        ubuntu@ip-172-31-61-205:~$ lscpu
        Architecture:          aarch64
        CPU op-mode(s):      32-bit, 64-bit
        Byte Order:          Little Endian
        CPU(s):                1
        On-line CPU(s) list: 0
        Vendor ID:             ARM
        Model:               1
        Thread(s) per core:  1
        Core(s) per socket:  1
        Socket(s):           1
        Stepping:            r1p1
        BogoMIPS:            2100.00
        Flags:               fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs pac
        a pacg dcpodp svei8mm svebf16 i8mm bf16 dgh rng
        Caches (sum of all):   
        L1d:                 64 KiB (1 instance)
        L1i:                 64 KiB (1 instance)
        L2:                  1 MiB (1 instance)
        L3:                  32 MiB (1 instance)
        NUMA:                  
        NUMA node(s):        1
        NUMA node0 CPU(s):   0
        Vulnerabilities:       
        Itlb multihit:       Not affected
        L1tf:                Not affected
        Mds:                 Not affected
        Meltdown:            Not affected
        Mmio stale data:     Not affected
        Retbleed:            Not affected
        Spec store bypass:   Mitigation; Speculative Store Bypass disabled via prctl
        Spectre v1:          Mitigation; __user pointer sanitization
        Spectre v2:          Mitigation; CSV2, BHB
        Srbds:               Not affected
        Tsx async abort:     Not affected
      ```
---
## History

--