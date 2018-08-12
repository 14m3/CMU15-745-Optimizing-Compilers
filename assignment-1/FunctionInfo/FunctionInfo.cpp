// 15-745 S18 Assignment 1: FunctionInfo.cpp
// Group: 14m3
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"

#include <iostream>
#include <string>

using namespace llvm;

namespace
{
    class FunctionInfo : public FunctionPass
    {
    public:
        static char ID;
        FunctionInfo() : FunctionPass(ID) { }
        ~FunctionInfo() { }

        // We don't modify the program, so we preserve all analyses
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.setPreservesAll();
        }

        // Do some initialization
        bool doInitialization(Module &M) override
        {
            outs() << "15745 Function Information Pass\n";
            outs() << "Name,\tArgs,\tCalls,\tBlocks,\tInsns\n";

            return false;
        }

        // Print output for each function
        bool runOnFunction(Function &F) override
        {
            //name
            StringRef FunctionName = F.getName();
            //args
            StringRef FunctionArgs{};
            if (F.isVarArg())
            {
                FunctionArgs = "*";
            }
            else
            {
                std::string tmp = std::to_string(F.arg_size());
                FunctionArgs = tmp;
            }
            //calls
            unsigned int FunctionCalledTimes = F.getNumUses();

            /*dumb method
            unsigned int FunctionCalledTimes = 0;
            for (Module::iterator fi = F.getParent()->begin(), fe = F.getParent()->end();
                fi != fe; ++fi)
            {
                Function* F_itr = &*fi;
                for (inst_iterator ii = inst_begin(F_itr), ie = inst_end(F_itr);
                    ii != ie; ++ii)
                {
                    Instruction* I = &*ii;
                    if (CallInst *CI = dyn_cast<CallInst>(I))
                    {
                        outs() << CI->getCalledFunction()->getName() << "\n";
                        if(CI->getCalledFunction() == &F)
                            FunctionCalledTimes++;
                    }
                }
            }
            */

            //bbs
            unsigned int FunctionBBNums = F.size();

            //insts
            unsigned int FunctionInstNums = 0;
            for (Function::iterator bi = F.begin(), be = F.end();
                bi != be; ++bi)
            {
                FunctionInstNums += bi->size();
            }
            
            outs() << FunctionName << ",\t" << FunctionArgs << ",\t" <<
                FunctionCalledTimes << ",\t" << FunctionBBNums << ",\t" <<
                FunctionInstNums << "\n";
            return false;
        }
    };

}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char FunctionInfo::ID = 0;
static RegisterPass<FunctionInfo> X("function-info", "15745: Function Information", false, false);
