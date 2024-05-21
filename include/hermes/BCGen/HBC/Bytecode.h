/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODE_H
#define HERMES_BCGEN_HBC_BYTECODE_H

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/BytecodeInstructionGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/BCGen/HBC/StringKind.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/BCGen/ShapeTableEntry.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Regex/RegexSerialization.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/StringTableEntry.h"
#include "hermes/Utils/Options.h"

#include "llvh/ADT/ArrayRef.h"

#include <memory>

namespace hermes {
class SourceMapGenerator;

namespace hbc {

// This class represents the in-memory representation of the bytecode function.
class BytecodeFunction {
  /// The bytecode executable. If this function contains switch statements, then
  /// the jump tables are also inlined in a 4-byte aligned block at the end of
  /// this vector.
  /// During execution, this vector can be executed directly because the vector
  /// storage will be 4-byte aligned.
  /// During serialisation, the jump tables and opcodes have to be separated so
  /// that the jump tables can be realigned based on the overall bytecode
  /// layout.
  const std::vector<opcode_atom_t> opcodesAndJumpTables_;

  /// Header that stores all the metadata of this function.
  FunctionHeader header_;

  /// Offsets of this function in debug info.
  DebugOffsets debugOffsets_{};

  /// Offset of the function info entry for this function, or 0 if none exists.
  uint32_t infoOffset = 0;

  /// List of exception handlers.
  std::vector<HBCExceptionHandlerInfo> exceptions_;

  /// Data to lazily compile this BytecodeFunction, if applicable.
  Function *lazyFunction_ = nullptr;

 public:
  /// Used during serialization. \p opcodes will be swapped after this call.
  explicit BytecodeFunction(
      std::vector<opcode_atom_t> &&opcodesAndJumpTables,
      FunctionHeader &&header,
      std::vector<HBCExceptionHandlerInfo> &&exceptionHandlers)
      : opcodesAndJumpTables_(std::move(opcodesAndJumpTables)),
        header_(std::move(header)),
        exceptions_(std::move(exceptionHandlers)) {}

  const FunctionHeader &getHeader() const {
    return header_;
  }

  FunctionHeaderFlag &mutableFlags() {
    return header_.flags;
  }

  void setOffset(uint32_t offset) {
    header_.offset = offset;
  }
  uint32_t getOffset() const {
    return header_.offset;
  }

  void setInfoOffset(uint32_t offset) {
    infoOffset = offset;
  }
  uint32_t getInfoOffset() const {
    return infoOffset;
  }

  bool isStrictMode() const {
    return header_.flags.strictMode;
  }

  /// Return the entire opcode array for execution, including the inlined jump
  /// tables.
  llvh::ArrayRef<opcode_atom_t> getOpcodeArray() const {
    return opcodesAndJumpTables_;
  }

  /// Return only the opcodes for serialisation. The jump tables need to be
  /// accessed separately so they can be correctly inlined.
  llvh::ArrayRef<opcode_atom_t> getOpcodesOnly() const {
    return {opcodesAndJumpTables_.data(), header_.bytecodeSizeInBytes};
  }

  /// Return the jump table portion of the opcode array. This is useful for the
  /// bytecode serialisation code when it is aligning the jump tables.
  llvh::ArrayRef<uint32_t> getJumpTablesOnly() const;

  uint32_t getExceptionHandlerCount() const {
    return exceptions_.size();
  }

  llvh::ArrayRef<HBCExceptionHandlerInfo> getExceptionHandlers() const {
    return exceptions_;
  }

  const DebugOffsets *getDebugOffsets() const {
    return &debugOffsets_;
  }

  void setDebugOffsets(DebugOffsets offsets) {
    debugOffsets_ = offsets;
    header_.flags.hasDebugInfo =
        debugOffsets_.sourceLocations != DebugOffsets::NO_OFFSET ||
        debugOffsets_.lexicalData != DebugOffsets::NO_OFFSET;
  }

  void setLazyFunction(Function *lazyFunction) {
    lazyFunction_ = lazyFunction;
  }
  Function *getLazyFunction() {
    return lazyFunction_;
  }

  /// \return true if the function should be compiled lazily.
  bool isLazy() const {
    return lazyFunction_ != nullptr;
  }
};

/// This class represents the in-memory representation of the bytecode module.
/// It contains pointers into Context, so it must not outlive the associated
/// Context.
class BytecodeModule {
  using FunctionList = std::vector<std::unique_ptr<BytecodeFunction>>;
  using SerializableBufferTy = std::vector<unsigned char>;

  /// A list of bytecode functions.
  FunctionList functions_{};

  /// Index of the top level function, code that gets executed in the global
  /// scope.
  uint32_t globalFunctionIndex_{};

  /// Run-length encoding representing the kinds of strings in the table.
  std::vector<StringKind::Entry> stringKinds_;

  /// The list of identifier hashes, corresponding to the string entries
  /// marked as identifiers, in order.
  std::vector<uint32_t> identifierHashes_;

  /// The global string table to represent each string in the string storage.
  StringLiteralTable stringTable_;

  /// The global bigint table for assigning IDs to bigints.
  bigint::UniquingBigIntTable bigIntTable_;

  /// The list of unique RegExp objects.
  UniquingRegExpTable regExpTable_;

  /// A table containing debug info (if compiled with -g).
  DebugInfo debugInfo_;

  /// Object/Array Literal Value Buffer table.
  SerializableBufferTy literalValueBuffer_;

  /// Object Key Buffer table.
  SerializableBufferTy objKeyBuffer_;

  /// Object shape table.
  std::vector<ShapeTableEntry> objShapeTable_;

  /// The segment ID corresponding to this BytecodeModule.
  /// This uniquely identifies this BytecodeModule within a set of modules
  /// which were compiled at the same time (and will correspond to a set of
  /// RuntimeModules in a Domain).
  uint32_t segmentID_;

  /// Table which indicates where to find the different CommonJS modules.
  /// Mapping from {filename ID => function index}.
  std::vector<std::pair<uint32_t, uint32_t>> cjsModuleTable_{};

  /// Table which indicates where to find the different CommonJS modules.
  /// Used if the modules have been statically resolved.
  /// Mapping from {global module ID => function index}.
  std::vector<std::pair<uint32_t, uint32_t>> cjsModuleTableStatic_{};

  /// Mapping function ids to the string table offsets that store their
  /// non-default source code representation that would be used by `toString`.
  /// These are only available when functions are declared with source
  /// visibility directives such as 'show source', 'hide source', etc.
  std::vector<std::pair<uint32_t, uint32_t>> functionSourceTable_{};

  /// Storing information about the bytecode, needed when it is loaded by the
  /// runtime.
  BytecodeOptions options_{};

 public:
  /// Create a simple BytecodeModule with only functionCount set.
  explicit BytecodeModule(uint32_t functionCount) {
    functions_.resize(functionCount);
  }

  /// Create an empty BytecodeModule with no functions.
  explicit BytecodeModule() = default;

  /// Add an uncompiled BytecodeFunction to the table, to track that we will
  /// eventually compile it.
  void addFunction() {
    functions_.push_back(nullptr);
  }
  const FunctionList &getFunctionTable() const {
    return functions_;
  }
  uint32_t getNumFunctions() const {
    return functions_.size();
  }

  void setGlobalFunctionIndex(uint32_t global) {
    globalFunctionIndex_ = global;
  }
  uint32_t getGlobalFunctionIndex() const {
    return globalFunctionIndex_;
  }

  /// Add a new bytecode function to the module at \p index.
  void setFunction(uint32_t index, std::unique_ptr<BytecodeFunction> F);

  BytecodeFunction &getFunction(unsigned index);

  BytecodeFunction &getGlobalCode() {
    return getFunction(globalFunctionIndex_);
  }

  /// Initialize the BytecodeModule with the given string table.
  /// Create the stringKinds_ and identifierHashes_ fields.
  void initializeStringTable(StringLiteralTable &&stringTable) {
    assert(stringTable_.empty() && "String table must be empty");
    stringKinds_ = stringTable.getStringKinds();
    identifierHashes_ = stringTable.getIdentifierHashes();
    stringTable_ = std::move(stringTable);
  }

  llvh::ArrayRef<StringKind::Entry> getStringKinds() const {
    return stringKinds_;
  }

  /// \return the number of identifiers.
  uint32_t getIdentifierCount() const {
    return identifierHashes_.size();
  }

  llvh::ArrayRef<uint32_t> getIdentifierHashes() const {
    return identifierHashes_;
  }

  unsigned getIdentifierID(llvh::StringRef str) const {
    return stringTable_.getIdentifierID(str);
  }
  unsigned getStringID(llvh::StringRef str) const {
    return stringTable_.getStringID(str);
  }

  uint32_t getStringTableSize() const {
    return stringTable_.count();
  }

  StringTableEntry::StringTableRefTy getStringTable() const {
    return stringTable_.getStringTableView();
  }

  uint32_t getStringStorageSize() const {
    return stringTable_.getStringStorageView().size();
  }

  StringTableEntry::StringStorageRefTy getStringStorage() const {
    return stringTable_.getStringStorageView();
  }

  uint32_t addBigInt(bigint::ParsedBigInt &&bigint) {
    return bigIntTable_.addBigInt(std::move(bigint));
  }

  llvh::ArrayRef<bigint::BigIntTableEntry> getBigIntTable() const {
    return bigIntTable_.getEntryList();
  }

  llvh::ArrayRef<uint8_t> getBigIntStorage() const {
    return bigIntTable_.getDigitsBuffer();
  }

  /// Add a new RegExp to the module.
  /// \param regExp the RegExp object to add, stored in the Context.
  uint32_t addRegExp(CompiledRegExp *regExp) {
    return regExpTable_.addRegExp(regExp);
  }

  llvh::ArrayRef<RegExpTableEntry> getRegExpTable() const {
    return regExpTable_.getEntryList();
  }

  llvh::ArrayRef<uint8_t> getRegExpStorage() const {
    return regExpTable_.getBytecodeBuffer();
  }

  void setSegmentID(uint32_t segmentID) {
    segmentID_ = segmentID;
  }
  uint32_t getSegmentID() const {
    return segmentID_;
  }

  void addCJSModule(uint32_t functionID, uint32_t nameID) {
    assert(
        cjsModuleTableStatic_.empty() &&
        "Statically resolved modules must be in cjsModulesStatic_");
    cjsModuleTable_.push_back({nameID, functionID});
  }

  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getCJSModuleTable() const {
    return cjsModuleTable_;
  }

  void addCJSModuleStatic(uint32_t moduleID, uint32_t functionID) {
    assert(
        cjsModuleTableStatic_.empty() &&
        "Statically resolved modules must be in cjsModulesStatic_");
    cjsModuleTableStatic_.push_back({moduleID, functionID});
  }

  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getCJSModuleTableStatic()
      const {
    return cjsModuleTableStatic_;
  }

  void addFunctionSource(uint32_t functionID, uint32_t stringID) {
    functionSourceTable_.push_back({functionID, stringID});
  }

  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getFunctionSourceTable() const {
    return functionSourceTable_;
  }

  DebugInfo &getDebugInfo() {
    return debugInfo_;
  }

  void setDebugInfo(DebugInfo info) {
    debugInfo_ = std::move(info);
  }

  /// Initialize the literal buffers.
  void initializeSerializedLiterals(
      std::vector<unsigned char> &&literalValueBuffer,
      std::vector<unsigned char> &&objKeyBuffer,
      std::vector<ShapeTableEntry> &&objShapeTable) {
    literalValueBuffer_ = std::move(literalValueBuffer);
    objKeyBuffer_ = std::move(objKeyBuffer);
    objShapeTable_ = std::move(objShapeTable);
  }

  /// Returns the amount of bytes in the object key buffer
  uint32_t getObjectKeyBufferSize() const {
    return objKeyBuffer_.size();
  }

  /// Returns the amount of bytes in the literal value buffer.
  uint32_t getLiteralValueBufferSize() const {
    return literalValueBuffer_.size();
  }

  /// Returns a reference to the literal value buffer.
  llvh::ArrayRef<unsigned char> getLiteralValueBuffer() const {
    return literalValueBuffer_;
  }

  /// Returns a reference to the object keys.
  llvh::ArrayRef<unsigned char> getObjectKeyBuffer() const {
    return llvh::ArrayRef<unsigned char>(objKeyBuffer_);
  }

  /// Returns a reference to the object shapes.
  llvh::ArrayRef<ShapeTableEntry> getObjectShapeTable() const {
    return objShapeTable_;
  }

  /// Returns a pair, where the first element is the key buffer starting at
  /// keyIdx and the second element is the value buffer starting at valIdx
  std::pair<llvh::ArrayRef<unsigned char>, llvh::ArrayRef<unsigned char>>
  getObjectBufferAtOffset(unsigned keyIdx, unsigned valIdx) const {
    return {
        llvh::ArrayRef<unsigned char>(objKeyBuffer_).slice(keyIdx),
        llvh::ArrayRef<unsigned char>(literalValueBuffer_).slice(valIdx)};
  }

  /// Populate the source map \p sourceMap with the debug information.
  void populateSourceMap(SourceMapGenerator *sourceMap) const;

  BytecodeOptions &getBytecodeOptionsMut() {
    return options_;
  }
  BytecodeOptions getBytecodeOptions() const {
    return options_;
  }
};

} // namespace hbc
} // namespace hermes

#endif
