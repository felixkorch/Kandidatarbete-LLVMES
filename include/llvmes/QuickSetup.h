#pragma once
#include "llvmes/LLVMHeaders.h"

namespace llvmes {

	struct QuickSetup {
		llvm::llvm_shutdown_obj shutdown;
		llvm::LLVMContext context;
		std::unique_ptr<llvm::Module> module;
		llvm::IRBuilder<> builder;

		QuickSetup()
			: module(std::make_unique<llvm::Module>("main", context))
			, builder(context)
		{}
	};

}