#include <iostream>

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"

#include "SymbolicInterpreter.h"

extern SymbolicInterpreter SI;

/*
 * Implement your transfer functions.
 */
extern "C" void __DSE_Alloca__(int R, int *Ptr) {
    Address addr(R);
    z3::expr memExpr = SI.getContext().int_val((char *) Ptr); // verify this is correct
    SI.getMemory().insert(std::make_pair(addr, memExpr));
}

extern "C" void __DSE_Store__(int *X) {
    z3::expr z3Value = SI.getStack().top();
    SI.getStack().pop();
    Address addr(X);

    if (!z3Value.is_numeral()) {
        std::string registerString = z3Value.to_string();
        registerString.erase(0, 1);
        int registerNumber = std::stoi(registerString);
        Address registerAddr(registerNumber);

        z3Value = SI.getMemory().at(registerAddr);
    }

    SI.getMemory().insert(std::make_pair(addr, z3Value));
}

extern "C" void __DSE_Load__(int Y, int *X) {
    Address registerAddr(Y);
    Address memAddr(X);
    z3::expr z3Value = SI.getMemory().at(memAddr);

    SI.getMemory().insert(std::make_pair(registerAddr, z3Value));
}

extern "C" void __DSE_ICmp__(int R, int Op) {
    z3::expr firstVal = SI.getStack().top();
    SI.getStack().pop();

    if (!firstVal.is_numeral()) {
        std::string registerString = firstVal.to_string();
        registerString.erase(0, 1);
        int registerNumber = std::stoi(registerString);
        Address registerAddr(registerNumber);

        firstVal = SI.getMemory().at(registerAddr);

    }

    z3::expr secondVal = SI.getStack().top();
    SI.getStack().pop();

    if (!secondVal.is_numeral()) {
        std::string registerString = secondVal.to_string();
        registerString.erase(0, 1);
        int registerNumber = std::stoi(registerString);
        Address registerAddr(registerNumber);

        secondVal = SI.getMemory().at(registerAddr);

    }

    z3::expr result = SI.getContext().int_val(0);
    if (Op == llvm::CmpInst::Predicate::ICMP_EQ) {
        result = secondVal == firstVal;
    } else if (Op == llvm::CmpInst::Predicate::ICMP_NE) {
        result = secondVal != firstVal;
    } else if (Op == llvm::CmpInst::Predicate::ICMP_SGE || Op == llvm::CmpInst::Predicate::ICMP_UGE) {
        result = secondVal >= firstVal;
    } else if (Op == llvm::CmpInst::Predicate::ICMP_SGT || Op == llvm::CmpInst::Predicate::ICMP_UGT) {
        result = secondVal > firstVal;
    } else if (Op == llvm::CmpInst::Predicate::ICMP_SLE || Op == llvm::CmpInst::Predicate::ICMP_ULE) {
        result = secondVal <= firstVal;
    } else if (Op == llvm::CmpInst::Predicate::ICMP_SLT || Op == llvm::CmpInst::Predicate::ICMP_ULT) {
        result = secondVal < firstVal;
    }

    Address registerAddr(R);
    SI.getMemory().insert(std::make_pair(registerAddr, result));
}

extern "C" void __DSE_BinOp__(int R, int Op) {
    z3::expr firstVal = SI.getStack().top();
    SI.getStack().pop();

    if (!firstVal.is_numeral()) {
        std::string registerString = firstVal.to_string();
        registerString.erase(0, 1);
        int registerNumber = std::stoi(registerString);
        Address registerAddr(registerNumber);

        firstVal = SI.getMemory().at(registerAddr);

    }

    z3::expr secondVal = SI.getStack().top();
    SI.getStack().pop();

    if (!secondVal.is_numeral()) {
        std::string registerString = secondVal.to_string();
        registerString.erase(0, 1);
        int registerNumber = std::stoi(registerString);
        Address registerAddr(registerNumber);

        secondVal = SI.getMemory().at(registerAddr);

    }

    z3::expr result = SI.getContext().int_val(0);
    if (Op == llvm::Instruction::Add) {
        result = secondVal + firstVal;
    } else if (Op == llvm::Instruction::Sub) {
        result = secondVal - firstVal;
    } else if (Op == llvm::Instruction::Mul) {
        result = secondVal * firstVal;
    } else if (Op == llvm::Instruction::SDiv) {
        result = secondVal / firstVal;
    } else if (Op == llvm::Instruction::UDiv) {
        result = secondVal / firstVal;
    }

    Address registerAddr(R);
    SI.getMemory().insert(std::make_pair(registerAddr, result));
}
