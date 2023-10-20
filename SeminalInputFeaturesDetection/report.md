# Project - Milestone-1

## Part2 Seminal Input Features Detection in LLVM

## Introduction

In our project, we aim to detect and analyze seminal input features in programs using LLVM passes. Seminal input features are those variables or elements in a program that are influenced by external input operations. Recognizing these features is vital for various use cases, including security analysis, debugging, and optimization.

## Current Pathway

Input Detection Pass: This pass detects and logs all input operations (like scanf, fopen, getc, etc.) in a program. It provides the foundation to understand which variables are influenced by input operations.
Def-Use Analysis Pass: This pass analyzes the definition-use chains for each key point in the program, especially branching points or function pointers. If a variable directly influences a key point, it's marked. If the influence is indirect (e.g., computed using another variable influenced by input), that's noted as well.

### Design

Input Detection Pass (InputDetectionPass.cpp)
Visitor Function: Goes through each instruction in a function and checks if it's a CallInst type.
Detection: If the called function is an input function (like scanf), it logs the detection.
Def-Use Analysis Pass (DefUseAnalysisPass.cpp)
Visitor Function: Goes through each instruction in a function.
Detection:
For branching instructions or function pointers, it analyzes the definition-use chain for each operand.
If an operand is influenced by an input function, this is logged.
Operands that are computed values (e.g., result of arithmetic operations) are further analyzed recursively.
### Test Files

hello.c: A simple program that takes input and checks a condition. This file serves as a testbed to validate our LLVM passes.

### Future Work

Semantics of I/O APIs: For complex scenarios (like fopen and getc), incorporate the semantics of the I/O operation. This would help understand how the size of a file or other such attributes influence the program's execution.
Integration with Build Systems: The project can be further integrated with build systems to automate the process of applying these passes during the build phase.


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
