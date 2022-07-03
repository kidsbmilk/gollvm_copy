
# Copyright 2018 The Go Authors. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

# Rules for building LLVMCppGoFrontEnd library, which incorporates
# both the Go frontend and the frontend -> LLVM-IR bridge component.
# 构建 LLVMCppGoFrontEnd 库的规则，该库包含 Go 前端和前端 -> LLVM-IR 桥组件。

# 这里的 CodeGen、Core、Support 是在哪里定义的？是如何查找的？感觉是通过哪里的 cmake 文件中的 for 循环批量设置的？TODO-ZZ
set(LLVM_LINK_COMPONENTS
  CodeGen
  Core
  Support
  )

# llvm-project/llvm/cmake/modules/AddLLVM.cmake 中定义了 add_llvm_library
add_llvm_library(LLVMCppGoFrontEnd
  ${GOFRONTEND_SOURCE_DIR}/ast-dump.cc
  ${GOFRONTEND_SOURCE_DIR}/embed.cc
  ${GOFRONTEND_SOURCE_DIR}/escape.cc
  ${GOFRONTEND_SOURCE_DIR}/export.cc
  ${GOFRONTEND_SOURCE_DIR}/expressions.cc
  ${GOFRONTEND_SOURCE_DIR}/go-diagnostics.cc
  ${GOFRONTEND_SOURCE_DIR}/go-encode-id.cc
  ${GOFRONTEND_SOURCE_DIR}/go-dump.cc
  ${GOFRONTEND_SOURCE_DIR}/go-optimize.cc
  ${GOFRONTEND_SOURCE_DIR}/go.cc
  ${GOFRONTEND_SOURCE_DIR}/gogo.cc
  ${GOFRONTEND_SOURCE_DIR}/import-archive.cc
  ${GOFRONTEND_SOURCE_DIR}/import.cc
  ${GOFRONTEND_SOURCE_DIR}/lex.cc
  ${GOFRONTEND_SOURCE_DIR}/names.cc
  ${GOFRONTEND_SOURCE_DIR}/parse.cc
  ${GOFRONTEND_SOURCE_DIR}/runtime.cc
  ${GOFRONTEND_SOURCE_DIR}/statements.cc
  ${GOFRONTEND_SOURCE_DIR}/types.cc
  ${GOFRONTEND_SOURCE_DIR}/unsafe.cc
  ${GOFRONTEND_SOURCE_DIR}/wb.cc
  go-backend.cpp
  go-llvm-bexpression.cpp
  go-llvm-bfunction.cpp
  go-llvm-bnode.cpp
  go-llvm-bstatement.cpp
  go-llvm-btype.cpp
  go-llvm-builtins.cpp
  go-llvm-bvariable.cpp
  go-llvm-cabi-oracle.cpp
  go-llvm-diagnostics.cpp
  go-llvm-dibuildhelper.cpp
  go-llvm-genblocks.cpp
  go-llvm-irbuilders.cpp
  go-llvm-linemap.cpp
  go-llvm-materialize.cpp
  go-llvm-tree-integrity.cpp
  go-llvm-typemanager.cpp
  go-llvm.cpp
  go-sha1.cpp
  namegen.cpp
)

add_dependencies(LLVMCppGoFrontEnd intrinsics_gen)
add_dependencies(LLVMCppGoFrontEnd libmpfr libmpc libgmp)

# llvm-project/llvm/tools/gollvm/CMakeLists.txt 中定义了 EXTINSTALLDIR、GOFRONTEND_SOURCE_DIR。
include_directories(${EXTINSTALLDIR}/include)
include_directories(${GOFRONTEND_SOURCE_DIR})

