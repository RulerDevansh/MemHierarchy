# C++ Cache Architecture Simulator 

## Overview
This repository contains a configurable, trace-driven L1 cache simulator implemented in C++. The project features a customizable cache architecture designed to evaluate the performance impact of different set-associativity levels, block sizes, and eviction policies on real-world program workloads.

This project was originally developed as a successful partner project for a Computer Systems Fundamentals class. It processed complex memory traces (such as `gcc` and `swim`) to calculate hit rates, miss penalties, and total processor cycles, demonstrating an understanding of memory hierarchy fundamentals.

## Architecture & Cache Design

The design is configurable to model various cache architectures:
* **`csim` (Simulator Engine):** Contains the core logic for parsing memory access traces and simulating cache behavior. The singular `csim.cpp` file contains the complete simulation state machine and is preserved as the original class submission.
* **Associativity & Structure:** Models direct-mapped, N-way set-associative, and fully associative structures.
* **Write Policies:** Supports `write-allocate` vs. `no-write-allocate` handling for cache misses, and `write-through` vs. `write-back` mechanisms for updating main memory.
* **Eviction Policies:** Implements both LRU (Least-Recently-Used) and FIFO (First-In, First-Out) replacement algorithms for set-associative and fully associative configurations.

## Performance & Trade-off Calculations

The simulator was used to conduct a systematic evaluation across various workloads to determine optimal cache configurations for performance versus hardware cost. Key calculations and findings included:

* **Associativity vs. Hardware Complexity:** Moving from a direct-mapped cache to a 2-way set associative cache drastically reduced conflict misses. However, increasing associativity from 4-way to 8-way provided less than a 1% hit rate improvement while requiring double the tag-matching comparators. **4-way associativity** offered the optimal hardware-to-performance ratio.
* **Block Size vs. Miss Penalty:** While larger block sizes (such as 64 bytes) maximized spatial locality and pushed hit rates above 99%, the 100-cycle latency of fetching larger blocks from main memory upon a miss outweighed the benefits. A **16-byte block size** consistently resulted in the lowest total execution cycles.
* **Eviction Policy Efficiency:** LRU's ability to preserve frequently accessed data proved much more effective than FIFO's age-based eviction, easily justifying the additional metadata overhead per cache line.

## Verification
The design includes in-depth verification against real-world memory access traces (`gcc.trace` and `swim.trace`) to validate cache hit/miss logic and cycle counting.
* The simulator reads parsed lines of memory accesses (loads and stores) and correctly identifies Hits, Misses, and Evictions.
* Automated tracking of cycles accounts for 1 cycle per cache access and 100 cycles per main memory block transfer.
* The synthesis of these trade-offs points to the following hardware specifications as the "Best Overall Cache" for a 16KB budget: **16 KB Total Size, 256 Sets, 4-Way Set Associative, 16-byte Blocks, Write-Allocate, Write-Back, LRU Eviction.**

## Repository Structure
* `csim.cpp` - Core C++ simulator source code and simulation state machine.
* `Makefile` - Build configuration (optimized for x86 Linux environments, e.g., university `ugrad` servers).
* `*.trace` - Real-world memory traces used for verification and performance analysis.
* `results.csv` - Empirical data gathered from trace simulations.

## Technologies & Tools
* **Programming Language:** C / C++
* **Concepts:** Memory Hierarchy, Associativity, LRU/FIFO Eviction, Write-back/Write-allocate policies.
* **Environment:** Tested and compiled on x86 Linux servers (University environments). 
