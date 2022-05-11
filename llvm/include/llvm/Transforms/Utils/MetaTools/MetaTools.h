#pragma once
#include "llvm/IR/PassManager.h"

namespace llvm 
{
	class MetaToolsPass : public PassInfoMixin<MetaToolsPass>
	{
	public:
		PreservedAnalyses run(Module& M, ModuleAnalysisManager& AM);
		static bool isRequired() { return true; }
	};

} // namespace llvm


#define MT_LLVM_VER 12