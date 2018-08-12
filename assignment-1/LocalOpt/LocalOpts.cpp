// 15-745 S18 Assignment 1: LocalOpts.cpp
// Group: 14m3
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include <string>

using namespace llvm;

namespace
{
    /*
    implement optimizations on basic blocks
    More details on local optimizations are available in Chapter 8.5 of the Purple Dragon Book
    */
    
    class LocalOpts : public BasicBlockPass
    {
    public:
        static char ID;
        LocalOpts() : BasicBlockPass(ID), algebraicTimes_(0), 
            constFoldTimes_(0), strengthReductTimes_(0) { }
        ~LocalOpts() { }

        // We don't modify the program, so we preserve all analyses
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            //AU.setPreservesAll();
        }

        // Do some initialization
        bool doInitialization(Function &F) override
        {
            outs() << "15745 Local Optimizations Pass\n";
            outs() << "Function: " << F.getName() << "\n";
            return false;
        }

        bool runOnBasicBlock(BasicBlock &BB) override
        {
            doAlgebraicIdentities(BB);
            doConstantFolding(BB);
            doStrengthReductions(BB);
            return false;
        }

        bool doFinalization(Function &F) override
        {
            printTransformationsAppliedDetail();
            return false;
        }

    private:
        void doAlgebraicIdentities(BasicBlock& BB);
        void doConstantFolding(BasicBlock& BB);
        void doStrengthReductions(BasicBlock& BB);
        void printTransformationsAppliedDetail();
        unsigned int algebraicTimes_;
        unsigned int constFoldTimes_;
        unsigned int strengthReductTimes_;
    };

}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char LocalOpts::ID = 0;
static RegisterPass<LocalOpts> X("local-opts", "15745: Local Optimizations", false, false);

void LocalOpts::printTransformationsAppliedDetail()
{
    outs() << "Transformations applied:\n";
    outs() << "\tAlgebraic identities: " << algebraicTimes_ <<"\n";
    outs() << "\tConstant folding: " << constFoldTimes_ <<"\n";
    outs() << "\tStrength reduction: " << strengthReductTimes_ <<"\n";
}

void LocalOpts::doAlgebraicIdentities(BasicBlock& BB)
{
    //outs() << "doAlgebraicIdentities\n";
    std::vector<Instruction*> InstToErase;
    for (auto& I: BB)
    {
        if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(&I))
        {
            Value* lhs = BinOp->getOperand(0);
            Value* rhs = BinOp->getOperand(1);
            ConstantInt* constLhs = dyn_cast<ConstantInt>(lhs);
            ConstantInt* constRhs = dyn_cast<ConstantInt>(rhs);
            switch(BinOp->getOpcode())
            {
                case Instruction::Add:
                {
                    // x + 0
                    if (constLhs && constLhs->isZero())
                    {
                        BinOp->replaceAllUsesWith(rhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    // 0 + x
                    if (constRhs && constRhs->isZero())
                    {
                        BinOp->replaceAllUsesWith(lhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    break;
                }
                case Instruction::Sub:
                {
                    // x - 0
                    if (constRhs && constRhs->isZero())
                    {
                        BinOp->replaceAllUsesWith(lhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    // x - x
                    if (lhs == rhs)
                    {
                        BinOp->replaceAllUsesWith(
                            ConstantInt::getSigned(BinOp->getType(), 0));
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    break;
                }
                case Instruction::Mul:
                {
                    // x * 0
                    if (constRhs && constRhs->isZero())
                    {
                        BinOp->replaceAllUsesWith(rhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    // 0 * x
                    if (constLhs && constLhs->isZero())
                    {
                        BinOp->replaceAllUsesWith(lhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    // x * 1
                    if (constRhs && constRhs->isOne())
                    {
                        BinOp->replaceAllUsesWith(lhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    // 1 * x
                    if (constLhs && constLhs->isOne())
                    {
                        BinOp->replaceAllUsesWith(rhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    break;
                }
                case Instruction::SDiv:
                case Instruction::UDiv:
                {
                    //0 / x
                    if (constLhs && constLhs->isZero())
                    {
                        BinOp->replaceAllUsesWith(lhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    //x / 1
                    if (constRhs && constRhs->isOne())
                    {
                        BinOp->replaceAllUsesWith(lhs);
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    if (lhs == rhs)
                    {
                        BinOp->replaceAllUsesWith(
                            ConstantInt::getSigned(BinOp->getType(), 1));
                        InstToErase.push_back(BinOp);
                        ++algebraicTimes_;
                        break;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    for (auto& I:InstToErase)
    {
        I->eraseFromParent();
    }
}

void LocalOpts::doConstantFolding(BasicBlock& BB)
{
    //outs() << "doConstantFolding\n";
    std::vector<Instruction*> InstToErase;
    for (auto& I: BB)
    {
        if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(&I))
        {
            Value* lhs = BinOp->getOperand(0);
            Value* rhs = BinOp->getOperand(1);
            ConstantInt* constLhs = dyn_cast<ConstantInt>(lhs);
            ConstantInt* constRhs = dyn_cast<ConstantInt>(rhs);
            switch(BinOp->getOpcode())
            {
                case Instruction::Add:
                {
                    if (constLhs && constRhs)
                    {
                        BinOp->replaceAllUsesWith(
                            ConstantInt::getSigned(BinOp->getType(), 
                                constLhs->getSExtValue() + constRhs->getSExtValue()));
                        InstToErase.push_back(BinOp);
                        ++constFoldTimes_;
                    }
                    break;
                }
                case Instruction::Sub:
                {
                    if (constLhs && constRhs)
                    {
                        outs() << constLhs->getSExtValue() << "\t" << 
                            constRhs->getSExtValue() << "\n";
                        BinOp->replaceAllUsesWith(
                            ConstantInt::getSigned(BinOp->getType(), 
                                constLhs->getSExtValue() - constRhs->getSExtValue()));
                        InstToErase.push_back(BinOp);
                        ++constFoldTimes_;
                    }
                    break;
                }
                case Instruction::Mul:
                {
                    if (constLhs && constRhs)
                    {
                        BinOp->replaceAllUsesWith(
                            ConstantInt::getSigned(BinOp->getType(), 
                                constLhs->getSExtValue() * constRhs->getSExtValue()));
                        InstToErase.push_back(BinOp);
                        ++constFoldTimes_;
                    }
                    break;
                }
                case Instruction::UDiv:
                case Instruction::SDiv:
                {
                    if (constLhs && constRhs)
                    {
                        BinOp->replaceAllUsesWith(
                            ConstantInt::getSigned(BinOp->getType(), 
                                constLhs->getSExtValue() / constRhs->getSExtValue()));
                        InstToErase.push_back(BinOp);
                        ++constFoldTimes_;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    for (auto& I:InstToErase)
    {
        I->eraseFromParent();
    }
}

void LocalOpts::doStrengthReductions(BasicBlock& BB)
{
    //outs() << "doStrengthReductions\n";
    std::vector<Instruction*> InstToErase;
    for (auto& I: BB)
    {
        if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(&I))
        {
            Value* lhs = BinOp->getOperand(0);
            Value* rhs = BinOp->getOperand(1);
            ConstantInt* constLhs = dyn_cast<ConstantInt>(lhs);
            ConstantInt* constRhs = dyn_cast<ConstantInt>(rhs);
            switch(BinOp->getOpcode())
            {
                case Instruction::Mul:
                {
                    // x * 2 -> x + x
                    if (constRhs && constRhs->equalsInt(2))
                    {
                        BinOp->replaceAllUsesWith(BinaryOperator::Create(
                                Instruction::Add, lhs, lhs, "", BinOp));
                        InstToErase.push_back(BinOp);
                        ++strengthReductTimes_;
                        break;
                    }
                    // 2 * x -> x + x
                    else if (constLhs && constLhs->equalsInt(2))
                    {
                        BinOp->replaceAllUsesWith(BinaryOperator::Create(
                                Instruction::Add, rhs, rhs, "", BinOp));
                        InstToErase.push_back(BinOp);
                        ++strengthReductTimes_;
                        break;
                    }
                    break;
                }
                case Instruction::UDiv:
                case Instruction::SDiv:
                {
                    // x/2 -> x >> 2
                    // x /2 != x >> 2
                    /*
                    if (constRhs && constRhs->equalsInt(2))
                    {
                        BinOp->replaceAllUsesWith(BinaryOperator::Create(
                            Instruction::AShr, 
                            lhs, ConstantInt::getSigned(lhs->getType(), 1), 
                            "", BinOp));
                        InstToErase.push_back(BinOp);
                        ++strengthReductTimes_;
                        break;
                    }
                    */
                    break;
                }
                default:
                    break;
            }
        }
    }
    for (auto& I:InstToErase)
    {
        I->eraseFromParent();
    }
}