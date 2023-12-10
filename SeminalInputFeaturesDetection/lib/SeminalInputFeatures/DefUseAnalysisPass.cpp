#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <set>
#include <fstream>
#include <queue>
#include <map>
#include <unordered_set>
using namespace llvm;
using Json = nlohmann::json;
struct VariableInfo {
    std::string name;
    int line;

    VariableInfo() : name(""), line(-1) {} // Default constructor
    VariableInfo(std::string n, int l) : name(n), line(l) {} // Parameterized constructor
};
namespace {


Json jsonInfluentialVariables;
    bool isInputFunction(const Function* F) {
        if (!F) {
            return false;
        }

        std::string funcName = F->getName().str();
        if (funcName.find("scanf") != std::string::npos) {
            return true;
        } else if (funcName.find("fopen") != std::string::npos) {
            return true;
        } else if (funcName.find("getc") != std::string::npos) {
            return true;
        }
        return false;
    }



    DbgDeclareInst* findDbgDeclare(Value* value, Function &F) {
        for (auto &BB : F) {
            for (auto &I : BB) {
                if (DbgDeclareInst *DbgDeclare = dyn_cast<DbgDeclareInst>(&I)) {
                    if (DbgDeclare->getAddress() == value) {
                        return DbgDeclare;
                    }
                }
            }
        }
        return nullptr;
    }

    void trackDefUseChain(Value *value, std::set<Value*>& visited, std::unordered_map<std::string, VariableInfo> &variableMap, Function &F) {
        if (!visited.insert(value).second) {
            return; 
        }

        if (Instruction *inst = dyn_cast<Instruction>(value)) {
            if (LoadInst *LoadInstVar = dyn_cast<LoadInst>(inst)) {
                Value *loadedValue = LoadInstVar->getPointerOperand();
               // errs() << "LoadInst found, tracking: " << *loadedValue << "\n";
                DbgDeclareInst *DbgDeclare = findDbgDeclare(loadedValue, F);
                if (DbgDeclare && DbgDeclare->getVariable()) {
                    std::string varName = DbgDeclare->getVariable()->getName().str();
                    int lineNo = DbgDeclare->getDebugLoc().getLine();
                  //  errs() << "Variable found from LoadInst: " << varName << " at line " << lineNo << "\n";
                    variableMap[varName] = VariableInfo(varName, lineNo);
                }
                trackDefUseChain(loadedValue, visited, variableMap, F);
            } else if (StoreInst *StoreInstVar = dyn_cast<StoreInst>(inst)) {
                Value *storedValue = StoreInstVar->getValueOperand();
                Value *storedLocation = StoreInstVar->getPointerOperand();
               
                trackDefUseChain(storedValue, visited, variableMap, F);
                trackDefUseChain(storedLocation, visited, variableMap, F);
            } else if (CallInst *CI = dyn_cast<CallInst>(inst)) {
          
                if (CI->getType() != Type::getVoidTy(F.getContext())) {
                  
                    trackDefUseChain(CI, visited, variableMap, F);  
                }

                for (auto arg = CI->arg_begin(); arg != CI->arg_end(); ++arg) {
                    Value *argValue = *arg;
                    trackDefUseChain(argValue, visited, variableMap, F);  
                }
                
            } else {
                for (unsigned i = 0; i < inst->getNumOperands(); ++i) {
                    trackDefUseChain(inst->getOperand(i), visited, variableMap, F);
                }
            }
        }
    }




    void analyzeTerminator(Value *value, std::set<Value*>& visited, std::unordered_map<std::string, VariableInfo> &variableMap, Function &F) {
    
        if (Instruction *inst = dyn_cast<Instruction>(value)) {
           
            for (unsigned i = 0; i < inst->getNumOperands(); ++i) {
               
                trackDefUseChain(inst->getOperand(i), visited,variableMap, F);
            }
        }
    }
    void analyzeLoop(Loop *loop,std::set<Value*>& visited, std::unordered_map<std::string, VariableInfo> &variableMap,Function &F) {
        
        BasicBlock *header = loop->getHeader();
        for (auto &I : *header) {
            if (auto *BI = dyn_cast<BranchInst>(&I)) {
                if (BI->isConditional()) {
                   
                    analyzeTerminator(BI->getCondition(), visited, variableMap, F);
                }
            }
        }
    }


    void visitor(Function &F,LoopInfo &LI) {


        std::vector<std::string> influentialVariables;
        std::set<Value*> visited;
        std::unordered_map<std::string, VariableInfo> variableMap;
        std::unordered_set<std::string> ioVariables;
        //step1: Find all loops.
        for (Loop *loop : LI) {
            analyzeLoop(loop, visited, variableMap, F);
        }
        
        //step2: Trace the source of all variables within the function.
        for (auto &BB : F) {
            for (auto &I : BB) {
                if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
                    DbgDeclareInst *DbgDeclare = findDbgDeclare(AI, F);
                    if (DbgDeclare && DbgDeclare->getVariable()) {
                        std::string varName = DbgDeclare->getVariable()->getName().str();
                        int lineNo = DbgDeclare->getDebugLoc().getLine();
                       // errs() << "Variable " << varName << " allocated at line " << lineNo << "\n";
                        variableMap[varName] = VariableInfo(varName, lineNo);
                    }
                }
                if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
                    Value *storedValue = SI->getValueOperand();
                    Value *storedLocation = SI->getPointerOperand();
                    trackDefUseChain(storedValue, visited, variableMap, F);
                    trackDefUseChain(storedLocation, visited, variableMap, F);
                }
            }
        }
      
        //step3: Search for input-related variables.
        for (auto &BB : F) {
            for (auto &I : BB) {
                if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                    Function *calledFunction = CI->getCalledFunction();
                   // errs() <<calledFunction->getName().str()<<'\n';
                    if (isInputFunction(calledFunction)) {
                        //errs() << "Find IO Function "<<calledFunction->getName().str()<<"\n";
                        
                        if (calledFunction->getName().str().find("fopen") != std::string::npos) {
                           
                            for (auto iter = CI->getIterator(); iter != BB.end(); ++iter) {
                                if (StoreInst *SI = dyn_cast<StoreInst>(&*iter)) {
                                    if (SI->getValueOperand() == CI) {
                                        Value *storedLocation = SI->getPointerOperand();
                                        DbgDeclareInst *DbgDeclare = findDbgDeclare(storedLocation, F);
                                        if (DbgDeclare && DbgDeclare->getVariable()) {
                                            std::string varName = DbgDeclare->getVariable()->getName().str();
                                            int lineNo = DbgDeclare->getDebugLoc().getLine();
                                            variableMap[varName] = VariableInfo(varName, lineNo);
                                            ioVariables.insert(varName);
                                           // errs() << "IO Variable " << varName << " used at line " << lineNo << "\n";
                                            break; 
                                        }
                                    }
                                }
                            }
                        } else {
                            // other IO functions
                            for (auto arg = CI->arg_begin(); arg != CI->arg_end(); ++arg) {
                                Value *argValue = *arg;
                                DbgDeclareInst *DbgDeclare = findDbgDeclare(argValue, F);
                                if (DbgDeclare && DbgDeclare->getVariable()) {
                                    std::string varName = DbgDeclare->getVariable()->getName().str();
                                    int lineNo = DbgDeclare->getDebugLoc().getLine();
                                    variableMap[varName] = VariableInfo(varName, lineNo);
                                    ioVariables.insert(varName); 
                                  //  errs() << "IO Variable " << varName << " used at line " << lineNo << "\n";
                                }
                            }
                        }
                    }
                }
            }
        }
        //step4: Match the termination condition variable with the input-related variable, and return the variable name and line number.
        if (!variableMap.empty()) {
            errs() << "Seminal Input Feature: ";
            bool ioVariableFound = false;
            for (const auto &entry : variableMap) {
                const VariableInfo &info = entry.second;
                // Check if the variable is an IO variable or indirectly affected by an IO variable.
                if (ioVariables.count(info.name) > 0) {
                    errs() << "Key variable: " << info.name << ", Line: " << info.line << "\n";
                    ioVariableFound = true;
                }
            }
            // If no IO variables were found, output all variables that could potentially influence loop termination
            if (!ioVariableFound) {
                for (const auto &entry : variableMap) {
                    const VariableInfo &info = entry.second;
                    errs() << "Potential influential variable: " << info.name << ", Line: " << info.line << "\n";
                }
            }
            errs() << "\n";
        } else {
            errs() << "No influential variables affected by external input detected.\n";
        }
        Json functionJson;
        functionJson["function"] = F.getName().str();
        Json variablesJson = Json::array();

        
        bool ioVariableFound = false;
        for (const auto &entry : variableMap) {
            const VariableInfo &info = entry.second;
            if (ioVariables.count(info.name) > 0) {
                Json variableJson;
                variableJson["name"] = info.name;
                variableJson["line"] = info.line;
                variableJson["type"] = "IO";
                variablesJson.push_back(variableJson);
                ioVariableFound = true;
            }
        }

        if (!ioVariableFound) {
            for (const auto &entry : variableMap) {
                const VariableInfo &info = entry.second;
                Json variableJson;
                variableJson["name"] = info.name;
                variableJson["line"] = info.line;
                variableJson["type"] = "Potential";
                variablesJson.push_back(variableJson);
            }
        }

        if (!variablesJson.empty()) {
            functionJson["influential_variables"] = variablesJson;
            jsonInfluentialVariables.push_back(functionJson);
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
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "def-use-analysis") {
                        FPM.addPass(DefUseAnalysisPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}
struct JsonFileWriter {
    ~JsonFileWriter() {
        std::ofstream file("influential_variables.json");
        file << jsonInfluentialVariables.dump(4);
        file.close();
    }
} jsonFileWriter;