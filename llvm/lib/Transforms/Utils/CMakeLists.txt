set(Utils_Groups
  MetaTools
	)

foreach(GroupName ${Utils_Groups})
  file(GLOB Group_HPP ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Utils/${GroupName}/*.h)
  file(GLOB Group_XPP ${GroupName}/*.*)
  file(GLOB Group_CPP ${GroupName}/*.cpp)
  message(Utils_Group:${GroupName})
  message(Utils_Group:${Group_HPP})
  foreach(XPPItem ${Group_XPP})
    message(Utils_Group:${XPPItem})
  endforeach()
  source_group("${GroupName}" FILES ${Group_XPP} ${Group_HPP})
endforeach()


add_llvm_component_library(LLVMTransformUtils
  AddDiscriminators.cpp
  AMDGPUEmitPrintf.cpp
  ASanStackFrameLayout.cpp
  AssumeBundleBuilder.cpp
  BasicBlockUtils.cpp
  BreakCriticalEdges.cpp
  BuildLibCalls.cpp
  BypassSlowDivision.cpp
  CallPromotionUtils.cpp
  CallGraphUpdater.cpp
  CanonicalizeAliases.cpp
  CanonicalizeFreezeInLoops.cpp
  CloneFunction.cpp
  CloneModule.cpp
  CodeExtractor.cpp
  CodeLayout.cpp
  CodeMoverUtils.cpp
  CtorUtils.cpp
  Debugify.cpp
  DemoteRegToStack.cpp
  EntryExitInstrumenter.cpp
  EscapeEnumerator.cpp
  Evaluator.cpp
  FixIrreducible.cpp
  FlattenCFG.cpp
  FunctionComparator.cpp
  FunctionImportUtils.cpp
  GlobalStatus.cpp
  GuardUtils.cpp
  HelloWorld.cpp
  InlineFunction.cpp
  InjectTLIMappings.cpp
  InstructionNamer.cpp
  IntegerDivision.cpp
  LCSSA.cpp
  LibCallsShrinkWrap.cpp
  Local.cpp
  LoopPeel.cpp
  LoopRotationUtils.cpp
  LoopSimplify.cpp
  LoopUnroll.cpp
  LoopUnrollAndJam.cpp
  LoopUnrollRuntime.cpp
  LoopUtils.cpp
  LoopVersioning.cpp
  LowerInvoke.cpp
  LowerMemIntrinsics.cpp
  LowerSwitch.cpp
  MatrixUtils.cpp
  MemoryOpRemark.cpp
  MemoryTaggingSupport.cpp
  Mem2Reg.cpp
  MetaRenamer.cpp
  ModuleUtils.cpp
  NameAnonGlobals.cpp
  PredicateInfo.cpp
  PromoteMemoryToRegister.cpp
  RelLookupTableConverter.cpp
  ScalarEvolutionExpander.cpp
  SCCPSolver.cpp
  StripGCRelocates.cpp
  SSAUpdater.cpp
  SSAUpdaterBulk.cpp
  SampleProfileInference.cpp
  SampleProfileLoaderBaseUtil.cpp
  SanitizerStats.cpp
  SimplifyCFG.cpp
  SimplifyIndVar.cpp
  SimplifyLibCalls.cpp
  SizeOpts.cpp
  SplitModule.cpp
  StripNonLineTableDebugInfo.cpp
  SymbolRewriter.cpp
  UnifyFunctionExitNodes.cpp
  UnifyLoopExits.cpp
  Utils.cpp
  ValueMapper.cpp
  VNCoercion.cpp
  ${Group_CPP}

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Utils
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Utils/MetaTools
  MetaTools

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Support
  )




