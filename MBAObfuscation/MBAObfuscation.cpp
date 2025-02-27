
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Obfuscation/Utils.h"
#include "llvm/Transforms/Obfuscation/MBAObfuscation.h"
#include "llvm/Transforms/Obfuscation/CryptoUtils.h"
#include "llvm/Transforms/Obfuscation/MBAUtils.h"
#include <vector>
#include <iostream>
using namespace std;
using namespace llvm;

bool MBAObfuscation::runOnFunction(Function &F){
    if(enable){
        INIT_CONTEXT(F);
        for(int i = 0;i < ObfuTimes;i ++){
            for(BasicBlock &BB : F){
                std::vector<Instruction*> origInst;
                for(Instruction &I : BB){
                    origInst.push_back(&I);
                }
                for(Instruction *I : origInst){
                    if(isa<BinaryOperator>(I)){
                        BinaryOperator *BI = cast<BinaryOperator>(I);
                        if(BI->getOperand(0)->getType()->isIntegerTy() && cryptoutils->get_uint8_t() % 100 < ObfuProb){
                            // Do not support 128-bit integers now
                            if(BI->getOperand(0)->getType()->getIntegerBitWidth() > 64){
                                continue;
                            }
                            substitute(BI);
                        }
                    }
                }
            }
        }
        return true;
    }
    return false;
}

void MBAObfuscation::substitute(BinaryOperator *BI){
    Value *mbaExpr = nullptr;
    switch (BI->getOpcode()) {
        case BinaryOperator::Add:
            mbaExpr = substituteAdd(BI);
            break;
        case BinaryOperator::Sub:
            mbaExpr = substituteSub(BI);
            break;
        case BinaryOperator::And:
            mbaExpr = substituteAnd(BI);
            break;
        case BinaryOperator::Or:
            mbaExpr = substituteOr(BI);
            break;
        case BinaryOperator::Xor:
            mbaExpr = substituteXor(BI);
            break;
        default:
            break;
    }
    if(mbaExpr){
        if(BI->getOperand(0)->getType()->getIntegerBitWidth() <= 32){
            mbaExpr = insertPolynomialMBA(mbaExpr, BI);
        }
        BI->replaceAllUsesWith(mbaExpr);
    }
}


Value* MBAObfuscation::substituteAdd(BinaryOperator *BI){
    int64_t *terms = generateLinearMBA(TermsNumber);
    terms[2] += 1;
    terms[4] += 1;
    return insertLinearMBA(terms, BI);
}

Value* MBAObfuscation::substituteSub(BinaryOperator *BI){
    int64_t *terms = generateLinearMBA(TermsNumber);
    terms[2] += 1;
    terms[4] -= 1;
    return insertLinearMBA(terms, BI);
}

Value* MBAObfuscation::substituteXor(BinaryOperator *BI){
    int64_t *terms = generateLinearMBA(TermsNumber);
    terms[5] += 1;
    return insertLinearMBA(terms, BI);
}

Value* MBAObfuscation::substituteAnd(BinaryOperator *BI){
    int64_t *terms = generateLinearMBA(TermsNumber);
    terms[0] += 1;
    return insertLinearMBA(terms, BI);
}

Value* MBAObfuscation::substituteOr(BinaryOperator *BI){
    int64_t *terms = generateLinearMBA(TermsNumber);
    terms[6] += 1;
    return insertLinearMBA(terms, BI);
}

FunctionPass* llvm::createMBAObfuscationPass(bool enable){
    return new MBAObfuscation(enable);
}

char MBAObfuscation::ID = 0;