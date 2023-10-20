/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_LOWERING_H
#define HERMES_BCGEN_LOWERING_H

#include "hermes/BCGen/RegAlloc.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/PassManager/PassManager.h"

namespace hermes {

Pass *createLIRPeephole();

/// Lowers switches to a sequence of IFs.
class SwitchLowering : public FunctionPass {
 public:
  explicit SwitchLowering() : FunctionPass("SwitchLowering") {}
  ~SwitchLowering() override = default;

  bool runOnFunction(Function *F) override;

 private:
  /// Replace the switch instruction with a sequence of IFs.
  void lowerSwitchIntoIfs(SwitchInst *switchInst);

  /// Copy a branch in all PhiInsts in a block.
  void
  copyPhiTarget(BasicBlock *block, BasicBlock *search, BasicBlock *replace);
  /// Erase incoming branches from all PhiInsts in a block.
  void erasePhiTarget(BasicBlock *block, BasicBlock *toDelete);
};

/// Lowers AllocObjects and its associated StoreOwnPropertyInst with literals
/// properties.
class LowerAllocObject : public FunctionPass {
 public:
  explicit LowerAllocObject() : FunctionPass("LowerAllocObject") {}
  ~LowerAllocObject() override = default;

  bool runOnFunction(Function *F) override;

 private:
  /// Define a type for managing lists of StoreNewOwnPropertyInsts.
  using StoreList = llvh::SmallVector<StoreNewOwnPropertyInst *, 4>;
  /// Define a type for mapping a given basic block to the stores to a given
  /// AllocObjectInst in that basic block.
  using BlockUserMap = llvh::DenseMap<BasicBlock *, StoreList>;

  /// Construct an ordered list of stores to \p allocInst that are known to
  /// always execute without any other intervening users.
  StoreList collectStores(
      AllocObjectInst *allocInst,
      const BlockUserMap &userBasicBlockMap,
      const DominanceInfo &DI);
  /// Serialize AllocObjects with literal property and value sets into object
  /// buffer; non-literals values could also be set as placeholders and later
  /// overwritten by PutByIds.
  bool lowerAllocObjectBuffer(
      AllocObjectInst *allocInst,
      const StoreList &users,
      uint32_t maxSize);
  /// Estimate best number of elements to serialize into the buffer.
  /// Try optimizing for max bytecode size saving.
  uint32_t estimateBestNumElemsToSerialize(const StoreList &users);
};

/// Lowers AllocObjectLiterals which target object literals with
/// constant properties.
class LowerAllocObjectLiteral : public FunctionPass {
 public:
  explicit LowerAllocObjectLiteral()
      : FunctionPass("LowerAllocObjectLiteral") {}
  ~LowerAllocObjectLiteral() override = default;

  bool runOnFunction(Function *F) override;

 private:
  uint32_t estimateBestNumElemsToSerialize(AllocObjectLiteralInst *allocInst);
  bool lowerAlloc(AllocObjectLiteralInst *allocInst);
  bool lowerAllocObjectBuffer(AllocObjectLiteralInst *allocInst);
};

/// Transform number-like string properties into a LiteralNumber.
class LowerNumericProperties : public FunctionPass {
 public:
  explicit LowerNumericProperties() : FunctionPass("LowerNumericProperties") {}
  ~LowerNumericProperties() override = default;

  bool runOnFunction(Function *F) override;

 private:
  bool stringToNumericProperty(
      IRBuilder &builder,
      Instruction &Inst,
      unsigned operandIdx);
};

// Limits the size of a function's array buffer by creating a
// StoreOwnPropertyInst for each element in an AllocArray once it reaches
// maxSize_ elements, since bytecode instructions can only represent up to
// a certain sized array.
// Also creates a StoreOwnPropertyInst for any `undefined` literal in an
// AllocArray, and all literals past it, since undefined cannot be added
// to the array buffer.
class LimitAllocArray : public FunctionPass {
 public:
  explicit LimitAllocArray(unsigned maxSizeInclusive)
      : FunctionPass("LimitAllocArray"), maxSize_(maxSizeInclusive) {}
  ~LimitAllocArray() override = default;
  bool runOnFunction(Function *F) override;

 private:
  uint32_t maxSize_;
};

/// Lowers conditional branches to CompareBranchInst instructions
class LowerCondBranch : public FunctionPass {
 public:
  explicit LowerCondBranch() : FunctionPass("LowerCondBranch") {}
  ~LowerCondBranch() override = default;
  bool runOnFunction(Function *F) override;

 private:
  /// \return whether the given binary operator can be lowered to a conditional
  /// branch.
  static bool isOperatorSupported(ValueKind kind);
};

/// Iterates over all instructions and performs lowering on exponentiation
/// operators to turn them into HermesInternal calls.
/// NOTE: It may be possible in the future to extend this pass to allow for
/// other lowering operations on single instructions.
class LowerExponentiationOperator : public FunctionPass {
 public:
  explicit LowerExponentiationOperator()
      : FunctionPass("LowerExponentiationOperator") {}
  ~LowerExponentiationOperator() override = default;
  bool runOnFunction(Function *F) override;

 private:
  /// Changes the binary exponentiation operator \p inst into a call to
  /// HermesInternal.exponentiationOperator.
  static bool lowerExponentiationOperator(
      IRBuilder &builder,
      BinaryOperatorInst *inst);
};

} // namespace hermes

#endif
