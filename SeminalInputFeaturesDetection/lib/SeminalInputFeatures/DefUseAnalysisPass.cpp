#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace {

    bool isInputFunction(const Function* F) {
        return F && (F->getName() == "scanf" || F->getName() == "fopen" || F->getName() == "getc");
    }

    void analyzeDefUseChain(Value* V, const std::string& indent = "") {
        errs() << indent << "- Operand '";
        if (V->hasName()) {
            errs() << V->getName();
        } else {
            V->print(errs());
        }
        errs() << "'";

        if (auto *inst = dyn_cast<Instruction>(V)) {
            if (auto *callInst = dyn_cast<CallInst>(inst)) {
                Function *calledFunc = callInst->getCalledFunction();
                if (isInputFunction(calledFunc)) {
                    errs() << " influenced by external input: " << calledFunc->getName() << " call in function " << inst->getParent()->getParent()->getName() << "\n";
                    return;
                }
            }

            errs() << " is a computed value.\n";
            for (auto &U : inst->operands()) {
                analyzeDefUseChain(U, indent + "  ");
            }
        } else {
            errs() << "\n";
        }
    }

    void visitor(Function &F) {
        errs() << "Analyzing function: " << F.getName() << "\n";
        
        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
            Instruction *Inst = &*I;

            if (auto *BI = dyn_cast<BranchInst>(Inst)) {
                if (BI->isConditional()) {
                    errs() << "Branch Instruction: " << *BI << "\n";
                    analyzeDefUseChain(BI->getCondition());
                }
            }
        }
    }


  
    struct DefUseAnalysisPass : PassInfoMixin<DefUseAnalysisPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
            
            visitor(F);
            return PreservedAnalyses::all();
        }
        static bool isRequired() { return true; }
    };

}  // end of anonymous namespace

llvm::PassPluginLibraryInfo getDefUseAnalysisPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "DefUseAnalysisPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                       
                        if (Name == "def-use-analysis") {
                            errs() << "def in"<< "\n";
                            FPM.addPass(DefUseAnalysisPass());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getDefUseAnalysisPluginInfo();
}
