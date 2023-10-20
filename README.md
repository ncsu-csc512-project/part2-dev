# Key Points Detection

## Overview
This README outlines the steps to compile and build the LLVM pass for branch and pointer profiling on UNIX systems.

Learn more [here](./objective.md).

## Prerequisites
- LLVM >= 16.0 installed
- CMake installed
- C++ compiler (e.g., g++, clang)




## Build the Pass

Clone the repo first if you haven't already:
```bash
git clone https://github.com/ncsu-csc512-project/part2-dev.git
```

Navigate to the root directory of the repo and run the following commands to build the pass:

```
export LLVM_DIR= # replace with your LLVM installation directory

mkdir build
cd build
cmake .. 
make
```

## Running the Pass
After building, you should have a `libDefUseAnalysisPass.so`and `libInputDetectionPass.so`  file in your `build` directory. To run the pass in the `tests` file, use LLVM's `opt` tool as follows:
```
./run_tests.sh
```

Replace `complex_branch_test.bc` with the LLVM IR file you want to analyze.



