# Proof of Concept Garbage Collector

Sample C/C++ Garbage Collector

This is super simple project is aimed to teach students how to build garbage collector from scratch with a lot of limitations in mind.

The example project should work on Xcode 10. Latest version also tested on Windows 10 with Visual Studio Community Edition 2017.

[Presentation sliides](https://drive.google.com/file/d/14Hzh0alo-Vpe1z1M4H9mHFACo_Qhv2Ap/view?usp=sharing)

## Limitations:
- single threaded
- 64-bit only (x86_64)
- single alloc < 4GB
- total amount of GC heap < 4GB


