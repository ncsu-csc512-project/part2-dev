#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/IRBuilder.h"


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

    void analyzeComplexControlFlow(Instruction *Inst, LoopInfo &LI) {
        errs() << "Analyzing Instruction: " << *Inst << "\n";

        // Check if the instruction is part of a loop.
        if (auto *loop = LI.getLoopFor(Inst->getParent())) {
            auto *header = loop->getHeader();
            // Output the loop header name or a placeholder if unnamed.
            StringRef loopHeaderName = header->hasName() ? header->getName() : "(unnamed loop header)";
            errs() << "Loop found with header: " << loopHeaderName << "\n";

            // Output the depth of the loop.
            errs() << "Loop depth: " << loop->getLoopDepth() << "\n";

            // Output all exiting blocks of the loop.
            SmallVector<BasicBlock*, 4> exitingBlocks;
            loop->getExitingBlocks(exitingBlocks);
            errs() << "Loop exiting blocks: ";
            for (auto *exitingBlock : exitingBlocks) {
                errs() << (exitingBlock->hasName() ? exitingBlock->getName() : "(unnamed)") << " ";
            }
            errs() << "\n";
        }

        // Check if the instruction is a switch statement.
        if (auto *SI = dyn_cast<SwitchInst>(Inst)) {
            errs() << "Switch statement found with " << SI->getNumCases() << " case(s).\n";
            for (auto Case : SI->cases()) {
                ConstantInt *caseVal = Case.getCaseValue();
                BasicBlock *caseDest = Case.getCaseSuccessor();
                errs() << "Case value: " << caseVal->getValue() << ", destination block: "
                    << (caseDest->hasName() ? caseDest->getName() : "(unnamed)") << "\n";
            }
        }
    }




    void visitor(Function &F, LoopInfo &LI) {
        errs() << "Analyzing function: " << F.getName() << "\n";
        
        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
            Instruction *Inst = &*I;

            if (auto *BI = dyn_cast<BranchInst>(Inst)) {
                if (BI->isConditional()) {
                  //  errs() << "Branch Instruction: " << *BI << "\n";
                    analyzeDefUseChain(BI->getCondition());
                }

            }
           // errs() << "Prepare Enter ControlFlow function! "  << "\n";
            analyzeComplexControlFlow(Inst,LI);
        }
    }
    
   
     
   
    struct DefUseAnalysisPass : PassInfoMixin<DefUseAnalysisPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
            visitor(F,LI); 
            return PreservedAnalyses::all();
        }
        static bool isRequired() { return true; }
    };



}  // end of anonymous namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "DefUseAnalysisPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            // Here we create a local FunctionAnalysisManager for the plugin
            FunctionAnalysisManager FAM;
            PB.registerFunctionAnalyses(FAM);

            PB.registerPipelineParsingCallback(
                [&](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "def-use-analysis") {
                        errs() << "Get in! " <<"\n";
                        FPM.addPass(DefUseAnalysisPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}

