#!/bin/bash

clang  -g -O0 -emit-llvm -c test_example1.c -o test_example1.bc
llvm-dis hello.bc -o test_example1.ll
opt -load-pass-plugin ../build/libDefUseAnalysisPass.so -passes=def-use-analysis -disable-output test_example1.bc

