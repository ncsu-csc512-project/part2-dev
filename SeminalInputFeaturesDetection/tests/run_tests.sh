#!/bin/bash

# Step 1: Compile hello.c into LLVM bitcode
clang -O0 -emit-llvm -c switch_test.c -o switch_test.bc

llvm-dis switch_test.bc -o switch_test.ll


#clang -emit-llvm -c complex_branch_test.c -o complex_branch_test.bc

# Step 2: Apply the passes
#opt -load-pass-plugin /Users/yukino/Documents/course/csc512/project/part2/SeminalInputFeaturesDetection/build/libInputDetectionPass.so -passes=input-detection -disable-output hello.bc
#opt -load-pass-plugin /Users/yukino/Documents/course/csc512/project/part2/SeminalInputFeaturesDetection/build/libDefUseAnalysisPass.so -passes=def-use-analysis -disable-output hello.bc
opt -load-pass-plugin ../build/libDefUseAnalysisPass.so -passes=def-use-analysis -disable-output switch_test.bc


# Optional steps are commented out for now
