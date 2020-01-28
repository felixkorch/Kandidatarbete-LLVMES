#include "llvmes/QuickSetup.h"

using namespace llvm;

Function* createFn1(Module* module, LLVMContext& context, IRBuilder<>& builder)
{
	FunctionCallee callee = module->getOrInsertFunction("add", builder.getInt32Ty(), builder.getInt32Ty());
	Function* add = dyn_cast<Function>(callee.getCallee()->stripPointerCasts());

	BasicBlock* entry = BasicBlock::Create(context, "entry", add);
	builder.SetInsertPoint(entry);

	Function::arg_iterator arg_it = add->arg_begin();
	Value* a = arg_it;
	Value* b = arg_it + 1;
	a->setName("a");
	b->setName("b");

	Value* result = builder.CreateAdd(a, b);
	builder.CreateRet(result);
	verifyFunction(*add, &llvm::outs());

	return add;
}

Function* createFn2(Module* module, LLVMContext& context, IRBuilder<>& builder)
{
	std::vector<Type*> args(2, builder.getInt32Ty());
	auto* fnType = FunctionType::get(builder.getInt32Ty(), args, false);

	Function* addFn = Function::Create(fnType, Function::ExternalLinkage, "add", *module);
	addFn->setCallingConv(CallingConv::C);

	Function::arg_iterator arg_it = addFn->arg_begin();
	Value* a = arg_it;
	Value* b = ++arg_it;
	a->setName("a");
	b->setName("b");

	BasicBlock* entry = BasicBlock::Create(context, "entry", addFn);
	builder.SetInsertPoint(entry);
	Value* result = builder.CreateAdd(a, b);
	builder.CreateRet(result);
	verifyFunction(*addFn, &llvm::outs());

	return addFn;
}

int main()
{
	llvmes::QuickSetup qs;
	createFn2(qs.module.get(), qs.context, qs.builder);
	qs.module->print(llvm::errs(), nullptr);
}