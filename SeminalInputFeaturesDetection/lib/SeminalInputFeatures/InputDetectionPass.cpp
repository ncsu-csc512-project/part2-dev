#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

    void visitor(Function &F) {
        errs() << "Analyzing function " << F.getName() << '\n';

        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
            Instruction *Inst = &*I;

            if (CallInst *callInst = dyn_cast<CallInst>(Inst)) {
                Function *calledFunc = callInst->getCalledFunction();
                if (calledFunc && calledFunc->getName() == "scanf") {
                    errs() << "Detected scanf call in function: " << F.getName() << "\n";
                }
                // TODO: Extend this for other input functions like fopen, getc, etc.
            }
        }
    }

    struct InputDetectionPass : PassInfoMixin<InputDetectionPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
            visitor(F);
            return PreservedAnalyses::all();
        }

        static bool isRequired() { return true; }
    };

}  // end of anonymous namespace

llvm::PassPluginLibraryInfo getInputDetectionPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "InputDetectionPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "input-detection") {
                            FPM.addPass(InputDetectionPass());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getInputDetectionPluginInfo();
}
