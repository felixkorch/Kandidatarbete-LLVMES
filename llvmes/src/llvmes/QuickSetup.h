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

using namespace llvm;

struct QuickSetup {
	llvm_shutdown_obj shutdown;
	LLVMContext context;
	std::unique_ptr<Module> module;
	IRBuilder<> builder;

	QuickSetup()
		: module(std::make_unique<Module>("main", context))
		, builder(context)
	{}
};