# Ray Tracing in One Weekend Series (Raylib C++)


<img src="./assets/inOneWeekend.gif" alt="Demo" width="600">


##  Overview

This repository contains my implementation of Peter Shirley's famous Ray Tracing book series, ported to C++ with **Raylib** for real-time visualization.

The goal is to move from a basic ray tracer to a full Monte Carlo path tracer.

| Project | Status | Description |
| :--- | :--- | :--- |
| **[Book 1: In One Weekend](./inOneWeekend)** | ✅ Completed | Basic spheres, materials (matte, metal, glass), camera, and defocus blur. |
| **[Book 2: The Next Week](./Book2)** | 🚧 Planned | Motion blur, BVH (optimization), image textures, Perlin noise, lights, and volumes. |
| **[Book 3: The Rest of Your Life](./Book3)** | 🚧 Planned | Monte Carlo integration, importance sampling, and advanced PDF management. |

##  Key Features of the Port
Unlike the original book code which outputs static PPM images, this implementation features:
- **Interactive Viewport:** Real-time navigation using Raylib.
- **Progressive Rendering:** Instant low-res preview while moving, high-quality accumulation when still.
- **Optimized for CPU:** Utilizing custom RNG and memory buffers for performance.

##  Build Instructions

This project uses **CMake** for build configuration.

### Prerequisites
* C++ Compiler (GCC, Clang, or MSVC)
* CMake (3.14+)
* Git

### Steps

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/megatr4n/RayTracingRayLib.git](https://github.com/megatr4n/RayTracingRayLib.git)
    cd RayTracingRayLib
    ```

2.  **Create build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure (IMPORTANT: Use Release Mode):**
    *Debug builds will be significantly slower due to the heavy math calculations required for ray tracing.*
    ```bash
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```

4.  **Build and Run:**
    ```bash
    cmake --build .
    ./RayTracer
    ```

Created by **Daniil Panasiuk(megatr4n)**
