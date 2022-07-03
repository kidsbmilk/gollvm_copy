
# Copyright 2018 The Go Authors. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

# Notes:
#
# The code in gofrontend depends on functions from GMP, MPC, and MPFR;
# these libraries are currently handled via the cmake "ExternalProject"
# utility.
#
# 项目开发中，有时需要在项目中使用第三方库。但是考虑到需要用到的第三方库太多，为了提高项目的可用性，
# 可以使用第三方导入的方式，即使用：ExternalProject_Add 指令。为了能使用 ExternalProject_Add 指令，
# 需要 include(ExternalProject) 导入插件。
include(ExternalProject)
# 确定处理器/内核的数量值并在 ${var} 中保存值，正的用法里，将处理器/内核的数量值保存在 PROCESSOR_COUNT 中。
include(ProcessorCount)
# 引入 llvm-project/llvm/cmake/modules/LLVMExternalProjectUtils.cmake
include(LLVMExternalProjectUtils)

# llvm-project/llvm/CMakeLists.txt 中有定义 CMAKE_MODULE_PATH
# CMAKE_CURRENT_SOURCE_DIR 是当前 CMakeLists.txt 所在路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
# 这个 CACHE 没有搞的太明白？TODO-ZZ
set(GOLLVM_USE_SPLIT_STACK ON CACHE BOOL "use split stack by default")
set(GOLLVM_DEFAULT_LINKER gold CACHE STRING "default linker for Go links")

# 在 llvm-project/llvm/tools/gollvm/cmake/modules 目录下，是由上面的 "list(APPEND CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules\")" 引入的目录。
include(CmakeUtils)
include(AddGollvm)

# So that we can issue "make -jN" cmds in externalproject_add（注意这个注释！）
processorcount(PROCESSOR_COUNT)

# CMAKE_CURRENT_BINARY_DIR：目标文件编译目录，比如：build.rel/tools/gollvm/external/install
# 这里的外部，指的是编译过程中下载的命令，比如：下面设置的 libgmp, libmpfr, libmpc
set(EXTINSTALLDIR ${CMAKE_CURRENT_BINARY_DIR}/external/install)
set(EXTLIBDIR "${EXTINSTALLDIR}/lib")
set(EXTINCLUDEDIR "${EXTINSTALLDIR}/include")

# 设置 cpp flags
set(EXTCPPFLAGS "CFLAGS=-I${EXTINCLUDEDIR}")
if(NOT "${CMAKE_SYSROOT}" STREQUAL "")
  # 官方的说明：CMAKE_SYSROOT：Path to pass to the compiler in the --sysroot flag.
  # 是哪里设置了 CMAKE_SYSROOT 呢？命令行 -D 指定的吗？TODO-ZZ
  string(APPEND EXTCPPFLAGS " --sysroot=${CMAKE_SYSROOT}")
endif()
if(LLVM_ENABLE_PIC)
  string(APPEND EXTCPPFLAGS " -fPIC -DPIC")
endif()

# 设置 ld flags
set(EXTLDFLAGS "LDFLAGS=-L${EXTLIBDIR}")

# 设置 cc，可以在命令行改成 clang 试试？TODO-ZZ
set(EXTCC "CC=${CMAKE_C_COMPILER}" "CXX=${CMAKE_CXX_COMPILER}")

set(gollvm_binroot "${CMAKE_CURRENT_BINARY_DIR}")

externalproject_add(libgmp
  URL https://gmplib.org/download/gmp/gmp-6.2.0.tar.bz2 https://mirrors.kernel.org/gnu/gmp/gmp-6.2.0.tar.bz2
  URL_MD5 c24161e0dd44cae78cd5f67193492a21
  # 路径：build.rel/tools/gollvm/external-downloads
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/external-downloads
  # 路径：build.rel/tools/gollvm/external/gmp-build
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/gmp-build
  # 路径：build.rel/tools/gollvm/external/gmp
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/gmp
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXTINSTALLDIR} ${EXTCPPFLAGS} ${EXTLDFLAGS} ${EXTCC}
  # make -j{数值} 表示最多允许多少个编译命令同时执行
  BUILD_COMMAND make -j${PROCESSOR_COUNT} install
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
  )

externalproject_add(libmpfr
  DEPENDS libgmp
  URL https://www.mpfr.org/mpfr-current/mpfr-4.1.0.tar.bz2 https://mirrors.kernel.org/gnu/mpfr/mpfr-4.1.0.tar.bz2
  URL_MD5 44b892bc5a45bafb4294d134e13aad1d
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/external-downloads
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/mpfr
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/mpfr
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --with-gmp=${CMAKE_CURRENT_BINARY_DIR}/external/gmp --prefix=${EXTINSTALLDIR} ${EXTCPPFLAGS} ${EXTLDFLAGS} ${EXTCC}
  BUILD_COMMAND make -j${PROCESSOR_COUNT} install
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
  )

externalproject_add(libmpc
  DEPENDS libgmp libmpfr
  URL https://ftp.gnu.org/gnu/mpc/mpc-1.2.0.tar.gz https://mirrors.kernel.org/gnu/mpc/mpc-1.2.0.tar.gz
  URL_MD5 2f1ce56ac775f2be090863f364931a03
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/external-downloads
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/mpc
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/mpc
  PREFIX ${EXTINSTALLDIR}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --with-gmp=${CMAKE_CURRENT_BINARY_DIR}/external/gmp --with-mpfr=${CMAKE_CURRENT_BINARY_DIR}/external/mpfr --prefix=${EXTINSTALLDIR} ${EXTCPPFLAGS} ${EXTLDFLAGS} ${EXTCC}
  BUILD_COMMAND make -j${PROCESSOR_COUNT} install
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
  )

# Top level targets for building, installing 这里是设定最终的编译、安装目标
# 全局搜索："add_dependencies(gollvm "，可以看到 gollvm 这个目标都做了啥。
# 我理解的CMake中的add_custom_target和add_custom_command：https://zhuanlan.zhihu.com/p/397394950
add_custom_target(gollvm ALL)
# 全局搜索："add_dependencies(install-gollvm "
add_custom_target(install-gollvm)

# In most use cases, we want to force a rebuild of all objects built
# from Go source if the compiler changes.
# llvm-project/llvm/tools/gollvm/driver-main/CMakeLists.txt 中定义生成：llvm-goc、llvm-goc-token。
set(gocdep llvm-goc llvm-goc-token)

# For compiler developers, however, the dependence this can be a
# hassle-- when hacking on the compiler, it is annoying to have each
# tiny change force a library rebuild, so the DISABLE_LIBGO_GOC_DEP can
# be set as an (unsafe) escape hatch to break the dependence from golibs
# to compiler. In this case we still need to insure that the compiler
# exists, but we don't care whether it is up to date or not.
# 这里的注释非常重要！TODO-ZZ
if (DISABLE_LIBGO_GOC_DEP)
  set(gocdep llvm-goc-token)
endif()

# Root of gollvm source code.
set(GOLLVM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Location of gofrontend source code.
set(GOFRONTEND_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gofrontend/go)

# Location of bridge source code.
set(BRIDGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bridge)

# Location of gollvm specific passes source code.
set(PASSES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/passes)

# Location of driver utilities source code.
set(DRIVER_UTILS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/driver)

# Subdirectory for the middle layer that translates Backend method
# calls into LLVM IR.
# 将后端方法调用转换为 LLVM IR 的中间层的子目录。
# add_subdirectory：将子目录添加到构建。source_dir 指定源 CMakeLists.txt 和代码文件所在的目录
add_subdirectory(bridge)

# Subdirectory for gollvm specific backend passes.
add_subdirectory(passes)

# Subdirectory for compiler driver utilities library.
add_subdirectory(driver)

# Subdirectory for compiler driver executable.
# 这里引入 driver-main/CMakeLists.txt，在其中会编译生成：llvm-goc，
# 这个 llvm-goc 其实就是后面编译各种 go 命令需要使用的 gocompiler。非常重要！
add_subdirectory(driver-main)

# Go standard library
# 下面两个的顺序还比较关键，先是处理 libgo 下的 CMakeLists.txt，会 include 其他文件，进而设置 shell，
# 下面的 gotools 的处理里会使用 shell 变量。
add_subdirectory(libgo)

# Go tools (go, gofmt, etc)
add_subdirectory(gotools)

# Subdir for unit tests
add_subdirectory(unittests)

# Top-level check target for gollvm
add_custom_target(check-gollvm DEPENDS check-libgo check-gotools)
