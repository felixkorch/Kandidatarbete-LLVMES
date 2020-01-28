#include "llvmes/QuickSetup.h"

using namespace llvm;

GlobalVariable* createGlobalVar(int size, const std::string& name, Module* module, IRBuilder<>& builder)
{
	Constant* globalInitValue = Constant::getIntegerValue(builder.getInt8Ty(), APInt(size, 0));
	GlobalVariable* globalVar = new GlobalVariable(*module,
		builder.getInt8Ty(), false, GlobalValue::LinkageTypes::ExternalLinkage, globalInitValue);
	globalVar->setName(name);
	return globalVar;
}

Function* LDA(Module* module, LLVMContext& context, IRBuilder<>& builder, int n)
{
	std::vector<Type*> args(1, builder.getInt8Ty());
	auto* fnType = FunctionType::get(builder.getVoidTy(), args, false);

	Function* LDA = Function::Create(fnType, Function::ExternalLinkage, "LDA", *module);
	LDA->setCallingConv(CallingConv::C);

	BasicBlock* entry = BasicBlock::Create(context, "entry", LDA);
	builder.SetInsertPoint(entry);

	Constant* loadValue = Constant::getIntegerValue(builder.getInt8Ty(), APInt(8, n));
	GlobalVariable* registerA = module->getGlobalVariable("registerA");
	builder.CreateStore(loadValue, registerA);
	builder.CreateRetVoid();
	verifyFunction(*LDA, &llvm::outs());

	return LDA;
}

int main()
{
	llvmes::QuickSetup qs;
	GlobalVariable* registerX = createGlobalVar(8, "registerX", qs.module.get(), qs.builder);
	GlobalVariable* registerY = createGlobalVar(8, "registerY", qs.module.get(), qs.builder);
	GlobalVariable* registerA = createGlobalVar(8, "registerA", qs.module.get(), qs.builder);
	GlobalVariable* flagN = createGlobalVar(8, "flagN", qs.module.get(), qs.builder);
	GlobalVariable* flagC = createGlobalVar(8, "flagC", qs.module.get(), qs.builder);
	GlobalVariable* flagZ = createGlobalVar(8, "flagZ", qs.module.get(), qs.builder);
	GlobalVariable* flagV = createGlobalVar(8, "flagV", qs.module.get(), qs.builder);

	Function* LDAInstruction = LDA(qs.module.get(), qs.context, qs.builder, 7);
	qs.module->print(llvm::errs(), nullptr);
}