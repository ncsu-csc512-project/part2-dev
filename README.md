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
export LLVM_DIR=" # replace with your LLVM installation directory

mkdir build
cd build
cmake .. 
make
```

## Running the Pass
After building, you should have a `libDefUseAnalysisPass.so`and `libInputDetectionPass.so`  file in your `build` directory. To run the pass, use LLVM's `opt` tool as follows:
```
run ./run_tests.sh
```

Replace `complex_branch_test.bc` with the LLVM IR file you want to analyze.

## Both .ll and .bc files are representations of LLVM's intermediate representation (IR).

However, they differ in their format:

.ll (LLVM Assembly Language Format):
This is a human-readable, textual representation of the LLVM IR.
You can open it with any text editor and read or modify it.
It's useful for debugging, understanding, and manually tweaking the IR.
The llvm-dis utility can be used to convert from the .bc format to the .ll format.

.bc (LLVM Bitcode Format):
This is a binary format representation of the LLVM IR.
It's not human-readable, so you can't open it in a text editor and understand it directly.
It's more compact and can be processed more efficiently by tools.
The llvm-as utility can be used to convert from the .ll format to the .bc format.
In summary, while both .ll and .bc represent LLVM IR, the former is a human-readable text format, and the latter is a compact binary format.