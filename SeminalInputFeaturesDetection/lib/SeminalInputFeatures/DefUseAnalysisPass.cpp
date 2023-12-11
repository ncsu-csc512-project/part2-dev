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
    /**
     * Function: isInputFunction
     * 
     * Description:
     * This function checks if a given LLVM Function is an input-related function, specifically looking for functions like 'scanf', 'fopen', and 'getc'. 
     * It is used to identify functions that are related to input operations in a program, which is crucial for analyzing the flow of data from input sources to key points in the program.
     * 
     * Inputs:
     * - const Function* F: A pointer to an LLVM Function object. This function is being checked to determine if it is an input-related function.
     * 
     * Output:
     * - Returns true if the function is identified as an input function ('scanf', 'fopen', or 'getc').
     * - Returns false if the function is not an input function or if the input Function pointer is null.
     * 
     * Implementation Details:
     * - The function first checks if the input Function pointer is null, returning false if it is. This is to ensure safe access to the Function object.
     * - It then retrieves the name of the function as a string.
     * - The function name is checked for the presence of substrings "scanf", "fopen", or "getc". If any of these are found, the function returns true, indicating it is an input-related function.
     * - If none of these substrings are found in the function name, it returns false.
     */

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


    /**
     * Function: findDbgDeclare
     * 
     * Description:
     * This function searches for a 'DbgDeclareInst' instance associated with a specific LLVM Value within a given function. 
     * 'DbgDeclareInst' is an LLVM instruction used in debugging information to connect a variable with its storage location. 
     * This function is particularly useful in analyzing variables' declarations and their metadata, which can include information such as the variable's name and line number in the source code.
     * 
     * Inputs:
     * - Value* value: The LLVM Value for which the corresponding 'DbgDeclareInst' is being searched. This value typically represents a variable or a memory location.
     * - Function &F: The LLVM Function within which the search is conducted. It contains the basic blocks and instructions to be inspected.
     * 
     * Output:
     * - Returns a pointer to the found 'DbgDeclareInst' if the corresponding declaration instruction for the input value is found within the function.
     * - Returns nullptr if no corresponding 'DbgDeclareInst' is found.
     * 
     * Implementation Details:
     * - The function iterates through each basic block (BB) of the function.
     * - In each basic block, it inspects each instruction (I).
     * - If the instruction is a 'DbgDeclareInst' and its associated address matches the input value, the function returns this 'DbgDeclareInst'.
     * - If no matching 'DbgDeclareInst' is found in the entire function, the function returns nullptr.
     */

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
    /**
     * Function: trackDefUseChain
     * 
     * Description:
     * This function recursively tracks the definition-use chain of a given LLVM Value within a function. It is designed to analyze the flow and dependencies of variables, identifying where they are defined and used. This analysis helps in understanding how different variables interact within the program, especially regarding their influence on key execution points like loops and conditional statements.
     * 
     * The function explores various types of instructions, such as Load, Store, and Call, to trace variables' origins and usages. It maintains a map of variable information, recording the name and line number of each variable encountered in the def-use chain. This map is later used to identify seminal input features of the function.
     * 
     * Inputs:
     * - Value *value: The initial LLVM Value (variable or instruction) from which the def-use chain tracking starts.
     * - std::set<Value*>& visited: A set to keep track of visited Values, avoiding redundant analysis.
     * - std::unordered_map<std::string, VariableInfo> &variableMap: A map to store information about each variable encountered during the tracking process.
     * - Function &F: The LLVM Function within which the analysis is conducted.
     * 
     * Outputs:
     * The function updates the 'variableMap' with information about the variables found along the def-use chain. It does not return any value.
     * 
     * Implementation Details:
     * - The function handles LoadInst by tracking the value loaded and adding its information to the map.
     * - For StoreInst, both the value being stored and the location where it is stored are tracked.
     * - CallInst are handled by tracking both the return value (if any) and the arguments of the function call.
     * - Recursion is used to explore all operands of an instruction, ensuring comprehensive coverage of the def-use chain.
     */

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

    /**
     * Function: visitor
     * 
     * Description:
     * This function analyzes a given LLVM function to identify seminal input features that influence key execution points, particularly loop termination conditions. It operates in several steps:
     * 1. Identifying and analyzing all loops in the function to trace variables influencing their termination.
     * 2. Tracing the source of all variables within the function using def-use chains to understand their origins and interactions.
     * 3. Searching for variables related to input operations, particularly focusing on standard IO functions and handling special cases like 'fopen'.
     * 4. Matching variables that influence loop terminations with those affected by input operations, to pinpoint seminal input features.
     * 
     * The function outputs a JSON-formatted summary of influential variables, classified as either direct IO variables or potential influencers. This output aids in understanding how inputs impact the program's critical execution points.
     * 
     * Inputs:
     * Function &F - A reference to the LLVM Function to be analyzed.
     * LoopInfo &LI - Loop information for the current function, used to identify and analyze loops.
     * 
     * Outputs:
     * The function does not return a value but prints out influential variables and generates a JSON file summarizing these variables.
     */


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