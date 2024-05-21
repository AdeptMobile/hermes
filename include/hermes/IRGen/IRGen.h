/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IRGEN_IRGEN_H
#define HERMES_IRGEN_IRGEN_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

#include <vector>

namespace hermes {

using DeclarationFileListTy = std::vector<ESTree::ProgramNode *>;

namespace sema {
class SemContext;
}

namespace flow {
class FlowContext;
}

/// Lowers an ESTree program into Hermes IR in \p M.
void generateIRFromESTree(
    Module *M,
    sema::SemContext &semCtx,
    flow::FlowContext &flowContext,
    ESTree::NodePtr node);

/// Lowers an ESTree program into Hermes IR in \p M.
void generateIRFromESTree(
    Module *M,
    sema::SemContext &semCtx,
    ESTree::NodePtr node);

/// Lowers an ESTree program into Hermes IR in \p M without a top-level
/// function, so that it can be used as a CommonJS module.
/// The same module can occur in any number of segments and all such copies of a
/// module are interchangeable at runtime, but the IR is not shared across them.
/// \param segmentID the ID of the segment containing this module.
/// \param id the ID assigned to the CommonJS module when added to the IR
///           (index when reading filenames for the first time).
/// \param filename the relative filename to the CommonJS module.
void generateIRForCJSModule(
    sema::SemContext &semContext,
    ESTree::FunctionExpressionNode *node,
    uint32_t segmentID,
    uint32_t id,
    llvh::StringRef filename,
    Module *M);

/// Generate IR for a lazy function.
/// Populates the module with any new Functions created as a result.
/// \param F the lazy function to lower, with a LazyCompilationDataInst as the
/// body.
/// \param node the AST node associated with the lazy function.
/// \param semCtx the semantic context for resolution.
/// \return the generated function.
Function *generateLazyFunctionIR(
    Function *F,
    ESTree::FunctionLikeNode *node,
    sema::SemContext &semCtx);

} // namespace hermes

#endif
