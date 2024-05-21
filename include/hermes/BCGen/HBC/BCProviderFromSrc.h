/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BCPROVIDERFROMSRC_H
#define HERMES_BCGEN_HBC_BCPROVIDERFROMSRC_H

#include "hermes/BCGen/HBC/BCProvider.h"
#include "hermes/BCGen/HBC/Bytecode.h"

#include "llvh/ADT/Optional.h"

namespace hermes {
namespace hbc {

/// Flags passed to createBCProviderFromSrc to set parameters on execution.
struct CompileFlags {
  bool debug{false};
  bool lazy{false};

  /// Eagerly compile files under this number of bytes, even when lazy.
  // Lazy compilation has significant per-module overhead, and is best applied
  // to large bundles with a lot of unused code. Eager compilation is more
  // efficient when compiling many small bundles with little unused code, such
  // as when the API user loads smaller chunks of JS code on demand.
  unsigned preemptiveFileCompilationThreshold{1 << 16};
  /// Eagerly compile functions under this number of bytes, even when lazy.
  unsigned preemptiveFunctionCompilationThreshold{160};

  bool strict{false};
  /// The value is optional; when it is set, the optimization setting is based
  /// on the value; when it is unset, it means the parser needs to automatically
  /// detect the 'use static builtin' directive and set the optimization setting
  /// accordingly.
  llvh::Optional<bool> staticBuiltins;
  bool verifyIR{false};
  /// If set, the compiler emits async break check instructions.  These may be
  /// used for several purposes, for example, to enforce a time limit on
  /// execution.  Other flags may also cause these instructions to be emitted,
  /// for example debugging.
  bool emitAsyncBreakCheck{false};
  /// Include libhermes declarations when compiling the file. This is done in
  /// normal compilation, but not for eval().
  bool includeLibHermes{true};
  /// Enable generators.
  bool enableGenerator{true};
  /// Enable ES6 classes support
  bool enableES6Classes{false};
  /// Define the output format of the generated bytecode. For instance, whether
  /// the bytecode is intended for execution or serialisation.
  OutputFormatKind format{Execute};
};

#ifndef HERMESVM_LEAN
/// BCProviderFromSrc is used when we are construction the bytecode from
/// source compilation, i.e. we generate BytecodeModule/BytecodeFunction
/// in this code path, and all the data are stored in those classes.
/// We should only depend on this when running from eval code path to
/// make sure maximum efficiency when loading from bytecode file.
class BCProviderFromSrc final : public BCProviderBase {
 public:
  /// The data needed to rerun on the compiler on more code that uses variables
  /// in this BCProvider.
  /// Copying this will increment the use count of shared pointers.
  class CompilationData {
   public:
    /// The options used to generate the BytecodeModule.
    BytecodeGenerationOptions genOptions;

    /// IR Module used for compiling more code into this BytecodeModule.
    /// May be shared with other BCProviders during local eval,
    /// which reuses SemContext and the IR but makes a new BCProvider.
    std::shared_ptr<Module> M;

    /// SemContext used for compiling more code into this BytecodeModule.
    /// May be shared with other BCProviders during local eval,
    /// which reuses SemContext and the IR but makes a new BCProvider.
    std::shared_ptr<sema::SemContext> semCtx;

    explicit CompilationData(
        const BytecodeGenerationOptions &genOptions,
        const std::shared_ptr<Module> &M,
        const std::shared_ptr<sema::SemContext> &semCtx)
        : genOptions(genOptions), M(M), semCtx(semCtx) {}
  };

 private:
  /// Data needed for compiling more code that uses the same
  /// variables/information.
  CompilationData compilationData_;

  /// The BytecodeModule that provides the bytecode data.
  /// Placed below CompilationData to ensure its destruction before the
  /// Module gets deleted.
  std::unique_ptr<hbc::BytecodeModule> module_;

  /// Whether the module constitutes a single function
  bool singleFunction_;

  /// Hash of all source files.
  SHA1 sourceHash_{SHA1{}};

  explicit BCProviderFromSrc(
      std::unique_ptr<hbc::BytecodeModule> &&module,
      CompilationData &&compilationData);

  /// No need to do anything since it's already created as part of
  /// BytecodeModule and set to the local member.
  void createDebugInfo() override {}

 public:
  /// Creates a BCProviderFromSrc by compiling the given JavaScript and
  /// optionally optimizing it with the supplied callback.
  /// \param buffer the JavaScript source to compile, encoded in utf-8. It is
  ///     required to have null termination ('\0') in the byte past the end,
  ///     in other words `assert(buffer.data()[buffer.size()] == 0)`.
  /// \param sourceURL this will be used as the "file name" of the buffer for
  ///     errors, stack traces, etc.
  /// \param sourceMap optional input source map for \p buffer.
  /// \param compileFlags self explanatory
  /// \param diagHandler handler for errors/warnings/notes.
  /// \param diagContext opaque data that will be passed to diagHandler.
  /// \param runOptimizationPasses if optimization is enabled in the settings
  ///     and this is non-null, invoke this callback with the IR module to
  ///     perform optimizations. This allows us to defer the decision of
  ///     whether to link all optimizations to the caller.
  /// \param defaultBytecodeGenerationOptions the starting bytecode generation
  ///     options that will be used during the bytecode generation phase.
  ///     Some options will be overriden depending on other arguments passed in.
  ///
  /// \return a BCProvider and an empty error, or a null BCProvider and an error
  ///     message (if diagHandler was provided, the error message is "error").
  static std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
  createBCProviderFromSrc(
      std::unique_ptr<Buffer> buffer,
      llvh::StringRef sourceURL,
      std::unique_ptr<SourceMap> sourceMap,
      const CompileFlags &compileFlags,
      SourceErrorManager::DiagHandlerTy diagHandler = {},
      void *diagContext = nullptr,
      const std::function<void(Module &)> &runOptimizationPasses = {},
      const BytecodeGenerationOptions &defaultBytecodeGenerationOptions =
          BytecodeGenerationOptions::defaults());

  /// Wrap an existing BytecodeModule in a BCProviderFromSrc.
  static std::unique_ptr<BCProviderFromSrc> createFromBytecodeModule(
      std::unique_ptr<hbc::BytecodeModule> bcModule,
      CompilationData &&compilationData) {
    auto result = std::unique_ptr<BCProviderFromSrc>(
        new BCProviderFromSrc(std::move(bcModule), std::move(compilationData)));
    result->module_->setBCProviderFromSrc(result.get());
    return result;
  }

  RuntimeFunctionHeader getFunctionHeader(uint32_t functionID) const override {
    return RuntimeFunctionHeader(&module_->getFunction(functionID).getHeader());
  }

  StringTableEntry getStringTableEntry(uint32_t index) const override {
    assert(index < stringCount_ && "invalid string table index");
    return module_->getStringTable()[index];
  }

  const uint8_t *getBytecode(uint32_t functionID) const override {
    return module_->getFunction(functionID).getOpcodeArray().data();
  }

  llvh::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t functionID) const override {
    return module_->getFunction(functionID).getExceptionHandlers();
  }

  const hbc::DebugOffsets *getDebugOffsets(uint32_t functionID) const override {
    return module_->getFunction(functionID).getDebugOffsets();
  }

  bool isFunctionLazy(uint32_t functionID) const override {
    return module_->getFunction(functionID).isLazy();
  }

  bool isLazy() const override {
    return false;
  }

  bool isSingleFunction() const {
    return singleFunction_;
  }

  hbc::BytecodeModule *getBytecodeModule() {
    return module_.get();
  }

  const BytecodeGenerationOptions &getBytecodeGenerationOptions() const {
    return compilationData_.genOptions;
  }

  Module *getModule() {
    return compilationData_.M.get();
  }

  sema::SemContext *getSemCtx() {
    return compilationData_.semCtx.get();
  }

  SHA1 getSourceHash() const override {
    return sourceHash_;
  };

  void setSourceHash(const SHA1 &hash) {
    sourceHash_ = hash;
  };

  static bool classof(const BCProviderBase *provider) {
    return provider->getKind() == BCProviderKind::BCProviderFromSrc;
  }
};

/// BCProviderLazy is used during lazy compilation. When a function is created
/// to be lazily compiled later, we create a BCProviderLazy object with
/// a pointer to such BytecodeFunction.
class BCProviderLazy final : public BCProviderBase {
  /// Pointer to the BytecodeFunction.
  hbc::BytecodeFunction *bytecodeFunction_;

  explicit BCProviderLazy(hbc::BytecodeFunction *bytecodeFunction);

  /// No debug information will be available without compiling it.
  void createDebugInfo() override {
    hermes_fatal("Accessing debug info from a lazy module");
  }

 public:
  static std::unique_ptr<BCProviderBase> createBCProviderLazy(
      hbc::BytecodeFunction *bytecodeFunction) {
    return std::unique_ptr<BCProviderBase>(
        new BCProviderLazy(bytecodeFunction));
  }

  RuntimeFunctionHeader getFunctionHeader(uint32_t) const override {
    return RuntimeFunctionHeader(&bytecodeFunction_->getHeader());
  }

  StringTableEntry getStringTableEntry(uint32_t index) const override {
    hermes_fatal("Accessing string table from a lazy module");
  }

  const uint8_t *getBytecode(uint32_t) const override {
    hermes_fatal("Accessing bytecode from a lazy module");
  }

  llvh::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t) const override {
    hermes_fatal("Accessing exception info from a lazy module");
  }

  const hbc::DebugOffsets *getDebugOffsets(uint32_t) const override {
    hermes_fatal("Accessing debug offsets from a lazy module");
  }

  bool isFunctionLazy(uint32_t) const override {
    return true;
  }

  bool isLazy() const override {
    return true;
  }

  /// \return the pointer to the BytecodeFunction.
  hbc::BytecodeFunction *getBytecodeFunction() {
    return bytecodeFunction_;
  }

  static bool classof(const BCProviderBase *provider) {
    return provider->getKind() == BCProviderKind::BCProviderLazy;
  }
};
#endif // HERMESVM_LEAN
} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BCPROVIDERFROMSRC_H
