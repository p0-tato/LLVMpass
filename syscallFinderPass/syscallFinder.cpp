/*
 * This LLVM pass searches the module for functions whose names contain a user-provided substring.
 * It then recursively traverses the call graph starting from each matching function to identify
 * potential syscall functions based on specific name patterns. The pass considers functions as
 * potential syscalls if their names include any of the following substrings:
 *   - "__ia32_compat_sys_"
 *   - "__x64_sys_"
 *   - "do_syscall"
 *   - "sys_" (as a prefix)
 *   - "__x32_compat_sys_"
 *   - "__arm64_sys_"
 *
 * When a potential syscall function is discovered during the traversal, its name is printed to the console.
 * This analysis is performed solely for information purposes and does not modify the module.
 */

#include "testk.h"
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/InlineAsm.h" 
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

char testk::ID;
using namespace llvm;

std::string target_name;
std::set<Function*> visited;

bool isPotentialSyscall(Function &F) {
    visited.insert(&F);
    if (F.getName().contains("__ia32_compat_sys_") || 
        F.getName().contains("__x64_sys_") ||
        F.getName().contains("do_syscall") || 
        F.getName().startswith("sys_") ||
        F.getName().contains("__x32_compat_sys_") ||
        F.getName().contains("__arm64_sys_")) {
        return true;
    }
    return false;
}

void findCallers(Function *targetFunction) {
    if (visited.find(targetFunction) != visited.end()) {
        return;
    }
    visited.insert(targetFunction);

    for (auto *user : targetFunction->users()) {
        if (auto *callInst = dyn_cast<CallInst>(user)) {
            Function *caller = callInst->getFunction();
            if (visited.find(caller) != visited.end()) {
                continue;
            }
            if (caller) {
                bool sys = isPotentialSyscall(*caller);
                if (sys) {
                    errs() << "[+] Callable - Found Syscall Function: " << caller->getName() << "\n";
                    return ;
                }
                findCallers(caller); 
            }
        }
    }
}

bool testk::runOnModule(Module &module) {
    errs() << "[+] runOnModule " << module.getName() << "\n";

    while (true) {
        std::string input;
        std::cout << "Enter name of Function (or 'exit' to quit): ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        bool foundFunction = false;
        visited.clear(); 

        for (Function &F : module) {
            if (F.getName().contains(input)) {
                foundFunction = true;
                target_name = F.getName().str();
                findCallers(&F);
            }
        }

        if (!foundFunction) {
            errs() << "[-] No function with the name " << input << " found in the module.\n";
        }
    }

    return false;
}

/*
static RegisterPass<testk>
    XXX("testk", "Test Kernel Pass");
*/