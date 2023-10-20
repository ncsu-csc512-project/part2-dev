#ifndef DEF_USE_ANALYSIS_PASS_H
#define DEF_USE_ANALYSIS_PASS_H

#include "llvm/Pass.h"

namespace llvm {
    class FunctionPass;
    class PassRegistry;

    void initializeDefUseAnalysisPassPass(PassRegistry&);
    FunctionPass* createDefUseAnalysisPass();
}

#endif
