# C++ & CUDA Systems Portfolio
### High-Performance Computing & Memory Management

## Overview
This repository documents my technical progression from foundational C++ through advanced memory management and into massively parallel computing with CUDA. As a graduate student in the Georgia Tech OMSCS program (Computing Systems specialization) with a structural background in Mechanical Engineering, my primary focus is bridging the gap between hardware constraints and software optimization.

The core objective of this active repository is the incremental development of a **custom CUDA-accelerated image processor**. The files enclosed represent the foundational building blocks for that project, moving from manual heap allocation and pointer manipulation to GPU kernel parallelization.

## Repository Structure & Core Concepts

### 1. `vector_test` (Advanced Memory Management)
* **Description:** A from-scratch implementation of a dynamically resizing `Vector` class, bypassing the standard library to directly interface with the operating system's heap memory.
* **Systems Concepts Demonstrated:**
  * **The Rule of Five:** Full implementation of the Destructor, Copy Constructor, Copy Assignment Operator, Move Constructor, and Move Assignment Operator.
  * **Move Semantics:** Optimizing temporary object returns by utilizing O(1) pointer stealing (`std::move`) to avoid expensive O(N) deep copies.
  * **RAII (Resource Acquisition Is Initialization):** Guaranteeing deterministic memory cleanup via strict destructor boundaries to eliminate memory leaks and dangling pointers.

### 2. `CUDA_Programming/` (Parallel Processing)
* **Description:** Initial host-to-device infrastructure utilizing the `nvcc` compiler via cloud-based NVIDIA hardware.
* **Systems Concepts Demonstrated:**
  * **Host vs. Device Coordination:** Synchronizing CPU execution with asynchronous GPU workloads (`cudaDeviceSynchronize`).
  * **Kernel Deployment:** Managing basic thread and block architecture configurations for parallel payload delivery.

### 3. Core C++ Architecture (`Pointers`, `SpaceInvaders`, `DoubleInt`)
* **Description:** Foundational exercises establishing strict type safety, memory address manipulation, and continuous application state.
* **Systems Concepts Demonstrated:**
  * **Raw Pointers vs. References:** Navigating the physical address space (`&`) and dereferencing (`*`) vs. safe object aliasing.
  * **State Management:** Maintaining continuous application loops and standard I/O stream extraction.

## Future Roadmap
* Transition from `.ipynb` prototyping environments to native `.cu` and `.cpp` files orchestrated by a `CMake` build system.
* Implementation of custom memory allocators to manage GPU VRAM efficiency.
* Construction of the parallelized image processing pipeline.