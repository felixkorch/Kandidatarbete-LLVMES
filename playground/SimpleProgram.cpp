#include "llvm/IR/Metadata.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Bitcode/BitcodeWriter.h"

///    Module should look like this:
///
///    int32 globalVar;
///    
///    int32 add(int32 a, int32 b)
///    {
///        return a + b;
///    }
///    
///    int32 plusFive()
///    {
///    	   return globalVar + 5;
///    }
///
///    int32 main()
///    {
///        printNumber(10);
///        return 0;
///    }
///
///

using namespace llvm;

Function* declareExtFn(Module* module, LLVMContext& context, IRBuilder<>& builder)
{
	std::vector<Type*> args(1, builder.getInt32Ty());
	auto* fnType = FunctionType::get(builder.getInt32Ty(), args, false);

	Function* extFn = Function::Create(fnType, Function::ExternalLinkage, "printNumber", *module);
	extFn->setCallingConv(CallingConv::C);
	return extFn;
}

Function* createMainFn(Module* module, LLVMContext& context, IRBuilder<>& builder)
{
	std::vector<Type*> args(0, builder.getInt32Ty());
	auto* fnType = FunctionType::get(builder.getInt32Ty(), args, false);

	Function* mainFn = Function::Create(fnType, Function::ExternalLinkage, "main", *module);
	mainFn->setCallingConv(CallingConv::C);

	BasicBlock* entry = BasicBlock::Create(context, "entry", mainFn);
	builder.SetInsertPoint(entry);

	Function* extFn = module->getFunction("printNumber");
	Constant* arg1 = Constant::getIntegerValue(builder.getInt32Ty(), APInt(32, 10));
	//Constant* arg2 = Constant::getIntegerValue(builder.getInt32Ty(), APInt(32, 11));

	builder.CreateCall(extFn, { arg1 });
	builder.CreateRet(Constant::getIntegerValue(builder.getInt32Ty(), APInt(32, 0)));

	verifyFunction(*mainFn, &llvm::outs());

	return mainFn;
}

Function* createAddFn(Module* module, LLVMContext& context, IRBuilder<>& builder)
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

Function* createPlusFiveFn(Module* module, LLVMContext& context, IRBuilder<>& builder)
{
	std::vector<Type*> args(0, builder.getInt32Ty());
	auto* fnType = FunctionType::get(builder.getInt32Ty(), args, false);

	Function* plusFiveFn = Function::Create(fnType, Function::ExternalLinkage, "plusFive", *module);
	plusFiveFn->setCallingConv(CallingConv::C);

	BasicBlock* entry = BasicBlock::Create(context, "entry", plusFiveFn);
	builder.SetInsertPoint(entry);
	GlobalVariable* globalVar = module->getGlobalVariable("globalVar");
	Value* result = builder.CreateLoad(globalVar);
	builder.CreateRet(result);
	verifyFunction(*plusFiveFn, &llvm::outs());

	return plusFiveFn;
}

int main()
{
	llvm_shutdown_obj shutdown; // obj that calls llvm_shutdown on destroy
	LLVMContext context;
	std::unique_ptr<Module> main = std::make_unique<Module>("main", context);
	IRBuilder<> builder(context);

	Constant* globalInitValue = Constant::getIntegerValue(builder.getInt32Ty(), APInt(32, 0));
	GlobalVariable* globalVar = new GlobalVariable(*main,
		builder.getInt32Ty(), false, GlobalValue::LinkageTypes::ExternalLinkage, globalInitValue);
	globalVar->setName("globalVar");

	Function* extFn = declareExtFn(main.get(), context, builder);
	Function* addFn = createAddFn(main.get(), context, builder);
	Function* plusFiveFn = createPlusFiveFn(main.get(), context, builder);
	Function* mainFn = createMainFn(main.get(), context, builder);
	main->print(llvm::errs(), nullptr);


	SMDiagnostic err;
	std::unique_ptr<Module> printModule = parseIRFile("PrintModule.ll", err, context);

	printModule->print(llvm::errs(), nullptr);

	Linker::linkModules(*main, std::move(printModule));
	
	std::error_code errCode;
	llvm::raw_fd_ostream out("MainModule.bc", errCode);
	WriteBitcodeToFile(*main, out);
}