#!/bin/bash

# Step 1: Compile hello.c into LLVM bitcode
clang -emit-llvm -c hello.c -o hello.bc
clang -emit-llvm -c complex_branch_test.c -o complex_branch_test.bc

# Step 2: Apply the passes
#opt -load-pass-plugin /Users/yukino/Documents/course/csc512/project/part2/SeminalInputFeaturesDetection/build/libInputDetectionPass.so -passes=input-detection -disable-output hello.bc
#opt -load-pass-plugin /Users/yukino/Documents/course/csc512/project/part2/SeminalInputFeaturesDetection/build/libDefUseAnalysisPass.so -passes=def-use-analysis -disable-output hello.bc
#  
opt -load-pass-plugin ../build/libDefUseAnalysisPass.so -passes=def-use-analysis -disable-output complex_branch_test.bc


