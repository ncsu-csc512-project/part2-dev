#ifndef INPUT_DETECTION_PASS_H
#define INPUT_DETECTION_PASS_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

class InputDetectionPass : public FunctionPass {
public:
    static char ID;  // Pass identification
    InputDetectionPass() : FunctionPass(ID) {}

    // Override FunctionPass methods
    bool runOnFunction(Function &F) override;
};

}  // namespace llvm

#endif  // INPUT_DETECTION_PASS_H
