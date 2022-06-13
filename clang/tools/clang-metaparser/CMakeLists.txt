set(LLVM_LINK_COMPONENTS
  Support
  )


set(ModuleList 
  MetaParser
  )



foreach(Module ${ModuleList})
	file(GLOB_RECURSE ${Module}_HPP ${Module}/*.h)
	file(GLOB_RECURSE ${Module}_CPP ${Module}/*.c*)
	list(APPEND AllModuleFiles ${${Module}_HPP} ${${Module}_CPP})
	
	file(GLOB Group_HPP ${Module}/*.h)
	file(GLOB Group_CPP ${Module}/*.c*)
	source_group("${Module}" FILES ${Group_CPP} ${Group_HPP})

	foreach(GroupName ${${Module}_Groups})
		file(GLOB Group_HPP ${GroupName}/*.h)
		file(GLOB Group_CPP ${GroupName}/*.c*)
		source_group("${GroupName}" FILES ${Group_CPP} ${Group_HPP})
	endforeach()
endforeach()

add_clang_executable(clang-metaparser main.cpp ${AllModuleFiles})

target_link_libraries(clang-metaparser
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )