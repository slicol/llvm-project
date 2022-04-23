set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_executable(clang-metatools main.cpp MetaDefines.cpp)

target_link_libraries(clang-metatools
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )