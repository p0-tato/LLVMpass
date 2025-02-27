#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

namespace llvm {
    class MBAObfuscation : public FunctionPass {
        public:
            static char ID;
            bool enable;

            MBAObfuscation(bool enable) : FunctionPass(ID) {
                this->enable = enable;
            }

            bool runOnFunction(Function &F);

            void substitute(BinaryOperator *BI);

            Value* substituteAdd(BinaryOperator *BI);
            Value* substituteSub(BinaryOperator *BI);
            Value* substituteAnd(BinaryOperator *BI);
            Value* substituteOr(BinaryOperator *BI);
            Value* substituteXor(BinaryOperator *BI);
    };

    FunctionPass* createMBAObfuscationPass(bool enable);
}