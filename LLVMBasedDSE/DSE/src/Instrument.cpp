#include "Instrument.h"

using namespace llvm;

namespace instrument {

Function *dseInitFunction;
Function *dseAllocaFunction;
Function *dseStoreFunction;
Function *dseLoadFunction;
Function *dseConstFunction;
Function *dseRegisterFunction;
Function *dseIcmpFunction;
Function *dseBranchFunction;
Function *dseBinOpFunction;
Type *intType;
Type *intPtrType;

void instrumentDseInit(Module *M, Function &F, Instruction *I) {

    IRBuilder<> builder(I);

    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), false);
    builder.CreateCall(functionType, dseInitFunction);
}

void instrumentAllocaInst(Module *M, Function &F, AllocaInst *I) {

    int registerId = getRegisterID(I);
    Value *registerValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), registerId);
    std::vector<Value *> args = {registerValue, I};
    ArrayRef<Value *> arrayRef(args);

    IRBuilder<> builder(I->getNextNode());
    std::vector<Type *> dseAllocaParamTypes = {intType, intPtrType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), dseAllocaParamTypes, false);
    builder.CreateCall(functionType, dseAllocaFunction, arrayRef);
}

void addConstInst(Module *M, Function &F, Instruction *I, Value *value) {

    std::vector<Value *> args = {value};
    ArrayRef<Value *> arrayRef(args);

    IRBuilder<> builder(I);
    std::vector<Type *> dseParamTypes = {intType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), dseParamTypes, false);
    builder.CreateCall(functionType, dseConstFunction, arrayRef);
}

void addRegisterInst(Module *M, Function &F, Instruction *I, Value *value) {
    int registerId = getRegisterID(value);
    Value *constantValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), registerId);
    std::vector<Value *> args = {constantValue};
    ArrayRef<Value *> arrayRef(args);

    IRBuilder<> builder(I);
    std::vector<Type *> dseParamTypes = {intType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), dseParamTypes, false);
    builder.CreateCall(functionType, dseRegisterFunction, arrayRef);
}

void instrumentStoreInst(Module *M, Function &F, StoreInst *I) {

    Value *value = I->getValueOperand();

    if (dyn_cast<Constant>(value)) {
        addConstInst(M, F, I, value);
    } else {
        addRegisterInst(M, F, I, value);
    }

    std::vector<Value *> args = {I->getPointerOperand()};
    ArrayRef<Value *> arrayRef(args);

    IRBuilder<> builder(I);
    std::vector<Type *> paramTypes = {intPtrType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), paramTypes, false);
    builder.CreateCall(functionType, dseStoreFunction, arrayRef);


}

void instrumentLoadInst(Module *M, Function &F, LoadInst *I) {
    int registerId = getRegisterID(I);
    Value *registerValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), registerId);
    std::vector<Value *> args = {registerValue, I->getPointerOperand()};
    ArrayRef<Value *> arrayRef(args);

    IRBuilder<> builder(I);
    std::vector<Type *> dseLoadParamTypes = {intType, intPtrType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), dseLoadParamTypes, false);
    builder.CreateCall(functionType, dseLoadFunction, arrayRef);

}

void instrumentBinOpInst(Module *M, Function &F, BinaryOperator *I) {

    Value *firstVal = I->getOperand(0);

    if (dyn_cast<Constant>(firstVal)) {
        addConstInst(M, F, I, firstVal);
    } else {
        addRegisterInst(M, F, I, firstVal);
    }

    Value *secondVal = I->getOperand(1);

    if (dyn_cast<Constant>(secondVal)) {
        addConstInst(M, F, I, secondVal);
    } else {
        addRegisterInst(M, F, I, secondVal);
    }

    int registerId = getRegisterID(I);
    Value *registerValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), registerId);
    Value *opcodeValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), I->getOpcode());
    std::vector<Value *> args = {registerValue, opcodeValue};
    ArrayRef<Value *> arrayRef(args);

    IRBuilder<> builder(I);
    std::vector<Type *> dseBinOpParamTypes = {intType, intType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), dseBinOpParamTypes, false);
    builder.CreateCall(functionType, dseBinOpFunction, arrayRef);
}

void instrumentCmpInst(Module *M, Function &F, CmpInst *I) {
    Value *firstVal = I->getOperand(0);

    if (dyn_cast<Constant>(firstVal)) {
        addConstInst(M, F, I, firstVal);
    } else {
        addRegisterInst(M, F, I, firstVal);
    }

    Value *secondVal = I->getOperand(1);

    if (dyn_cast<Constant>(secondVal)) {
        addConstInst(M, F, I, secondVal);
    } else {
        addRegisterInst(M, F, I, secondVal);
    }

    int registerId = getRegisterID(I);
    Value *registerValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), registerId);
    Value *opcodeValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), I->getPredicate());
    std::vector<Value *> args = {registerValue, opcodeValue};
    ArrayRef<Value *> arrayRef(args);

    IRBuilder<> builder(I);
    std::vector<Type *> dseBinOpParamTypes = {intType, intType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), dseBinOpParamTypes, false);
    builder.CreateCall(functionType, dseIcmpFunction, arrayRef);
}

void instrumentBranchInst(Module *M, Function &F, BranchInst *I) {

    if (!I->isConditional()) {
        return;
    }

    IRBuilder<> builder(I);

    int condRegisterId = getRegisterID(I->getCondition());
    Value *registerValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), condRegisterId);

    int branchId = getBranchID(I);
    Value *branchValue = ConstantInt::get(Type::getInt32Ty(I->getContext()), branchId);

    Value *condValue = builder.CreateIntCast(I->getCondition(), Type::getInt32Ty(I->getContext()), true);

    std::vector<Value *> args = {branchValue, registerValue, condValue};
    ArrayRef<Value *> arrayRef(args);

    std::vector<Type *> dseBranchParamTypes = {intType, intType, intType};
    FunctionType *functionType = FunctionType::get(Type::getVoidTy(F.getContext()), dseBranchParamTypes, false);
    builder.CreateCall(functionType, dseBranchFunction, arrayRef);
}

void instrumentInstIfNeeded(Module *M, Function &F, Instruction *I) {

    if (auto allocaInst = dyn_cast<AllocaInst>(I)) {
        instrumentAllocaInst(M, F, allocaInst);
    } else if (auto storeInst = dyn_cast<StoreInst>(I)) {
        instrumentStoreInst(M, F, storeInst);
    } else if (auto loadInst = dyn_cast<LoadInst>(I)) {
        instrumentLoadInst(M, F, loadInst);
    } else if (auto binOpInst = dyn_cast<BinaryOperator>(I)) {
        instrumentBinOpInst(M, F, binOpInst);
    } else if (auto cmpInst = dyn_cast<CmpInst>(I)) {
        instrumentCmpInst(M, F, cmpInst);
    } else if (auto branchInst = dyn_cast<BranchInst>(I)) {
        instrumentBranchInst(M, F, branchInst);
    }
}

/*
 * Implement your instrumentation for dynamic symbolic execution engine
 */
bool Instrument::runOnFunction(Function &F) {

    bool dseInitInstrumented = false;

	Module *module = F.getParent();

	// Add function declarations

    intType = Type::getInt32Ty(F.getContext());
    intPtrType = Type::getInt32PtrTy(F.getContext());

    insertInitFunctionDeclaration(F, module);
    insertAllocaFunctionDeclaration(F, module);
    insertStoreFunctionDeclaration(F, module);
    insertLoadFunctionDeclaration(F, module);
    insertConstFunctionDeclaration(F, module);
    insertRegisterFunctionDeclaration(F, module);
    insertIcmpFunctionDeclaration(F, module);
    insertBranchFunctionDeclaration(F, module);
    insertBinOpFunctionDeclaration(F, module);


    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {

        Instruction *inst = &*I;

        if (!dseInitInstrumented && F.getName() == "main") {
            instrumentDseInit(module, F, inst);
            dseInitInstrumented = true;
        }

        instrumentInstIfNeeded(module, F, inst);

    }

  	return true;
}

void Instrument::insertInitFunctionDeclaration(const Function &F, Module *module) {

    FunctionType *dseFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), false);
    StringRef dseFunctionName(DSEInitFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, dseFunctionType);
    dseInitFunction = cast<Function>(dseFunctionCallee.getCallee());
}

void Instrument::insertAllocaFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> dseAllocaParamTypes = {intType, intPtrType};
    FunctionType *dseAllocaFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), dseAllocaParamTypes, false);
    StringRef dseAllocaFunctionName(DSEAllocaFunctionName);
    FunctionCallee dseAllocaFunctionCallee = module->getOrInsertFunction(dseAllocaFunctionName, dseAllocaFunctionType);
    dseAllocaFunction = cast<Function>(dseAllocaFunctionCallee.getCallee());
}

void Instrument::insertStoreFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> paramTypes = {intPtrType};
    FunctionType *functionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), paramTypes, false);
    StringRef dseFunctionName(DSEStoreFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, functionType);
    dseStoreFunction = cast<Function>(dseFunctionCallee.getCallee());
}

void Instrument::insertLoadFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> dseParamTypes = {intType, intPtrType};
    FunctionType *dseFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), dseParamTypes, false);
    StringRef dseFunctionName(DSELoadFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, dseFunctionType);
    dseLoadFunction = cast<Function>(dseFunctionCallee.getCallee());
}

void Instrument::insertConstFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> dseParamTypes = {intType};
    FunctionType *dseFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), dseParamTypes, false);
    StringRef dseFunctionName(DSEConstFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, dseFunctionType);
    dseConstFunction = cast<Function>(dseFunctionCallee.getCallee());
}

void Instrument::insertRegisterFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> dseParamTypes = {intType};
    FunctionType *dseAllocaFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), dseParamTypes, false);
    StringRef dseFunctionName(DSERegisterFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, dseAllocaFunctionType);
    dseRegisterFunction = cast<Function>(dseFunctionCallee.getCallee());
}

void Instrument::insertIcmpFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> dseParamTypes = {intType, intType};
    FunctionType *dseFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), dseParamTypes, false);
    StringRef dseFunctionName(DSEICmpFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, dseFunctionType);
    dseIcmpFunction = cast<Function>(dseFunctionCallee.getCallee());
}

void Instrument::insertBranchFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> dseParamTypes = {intType, intType, intType};
    FunctionType *dseFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), dseParamTypes, false);
    StringRef dseFunctionName(DSEBranchFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, dseFunctionType);
    dseBranchFunction = cast<Function>(dseFunctionCallee.getCallee());
}

void Instrument::insertBinOpFunctionDeclaration(const Function &F, Module *module) {

    std::vector<Type *> dseParamTypes = {intType, intType};
    FunctionType *dseFunctionType = FunctionType::get
            (Type::getVoidTy(F.getContext()), dseParamTypes, false);
    StringRef dseFunctionName(DSEBinOpFunctionName);
    FunctionCallee dseFunctionCallee = module->getOrInsertFunction(dseFunctionName, dseFunctionType);
    dseBinOpFunction = cast<Function>(dseFunctionCallee.getCallee());
}



char Instrument::ID = 1;
static RegisterPass<Instrument>
    X("Instrument", "Instrumentations for Dynamic Symbolic Execution", false,
      false);

} // namespace instrument
