//===-- ToolChain.cpp -----------------------------------------------------===//
//
// Copyright 2018 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//
//===----------------------------------------------------------------------===//
//
// Gollvm driver helper class ToolChain methods.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/raw_ostream.h"

#include "Action.h"
#include "Driver.h"
#include "Tool.h"
#include "ToolChain.h"
#include "ReadStdin.h"

namespace gollvm {
namespace driver {

ToolChain::ToolChain(Driver &driver,
                     const llvm::Triple &targetTriple)
    : driver_(driver),
      triple_(targetTriple)
{
}

ToolChain::~ToolChain()
{
}

// 真正的得到编译器！
Tool *ToolChain::getCompiler()
{
  if (compiler_.get() == nullptr)
    // Driver::setup() 中会设置 toolchains_，目前只有 Linux 类型的。
    // 所以下面的 buildCompiler 方法在 llvm/tools/gollvm/driver/LinuxToolChain.cpp 中有实现。非常重要！
    compiler_.reset(buildCompiler());
  return compiler_.get();
}

Tool *ToolChain::getAssembler()
{
  if (assembler_.get() == nullptr)
    assembler_.reset(buildAssembler());
  return assembler_.get();
}

Tool *ToolChain::getLinker()
{
  if (linker_.get() == nullptr)
    linker_.reset(buildLinker());
  return linker_.get();
}

Tool *ToolChain::getStdinReader(bool mustBeEmpty)
{
  if (stdinReader_.get() == nullptr)
    stdinReader_.reset(new ReadStdin(*this, mustBeEmpty));
  return stdinReader_.get();
}

Tool *ToolChain::getTool(Action *act)
{
  assert(act != nullptr);
  switch(act->type()) {
    case Action::A_Compile:
    case Action::A_CompileAndAssemble:
      // 真正的得到编译器！
      return getCompiler();
    case Action::A_Assemble:
      return getAssembler();
    case Action::A_Link:
      return getLinker();
    case Action::A_ReadStdin: {
      ReadStdinAction *rsia = act->castToReadStdinAction();
      return getStdinReader(strcmp(rsia->suffix(), "go"));
    }
    default:
      assert(false);
      return nullptr;
  }
  return nullptr;
}

std::string ToolChain::getFilePath(const char *afile)
{
  return driver_.getFilePath(afile, thisToolChain());
}

std::string ToolChain::getProgramPath(const char *atool)
{
  return driver_.getProgramPath(atool, thisToolChain());
}

} // end namespace driver
} // end namespace gollvm
