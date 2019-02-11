/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_GCSEGMENTRANGE_H
#define HERMES_VM_GCSEGMENTRANGE_H

#include "hermes/Support/ConsumableRange.h"
#include "hermes/VM/AlignedHeapSegment.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Compiler.h"

#include <memory>
#include <vector>

namespace hermes {
namespace vm {

/// An interface representing a sequence of segments, and a collection of
/// implementations and factory functions for combining and manipulating them.
struct GCSegmentRange {
  using Ptr = std::unique_ptr<GCSegmentRange>;

  /// Factory function for creating a range from an iterator ranging over a
  /// sequence of AlignedHeapSegments. Provided for convenenience as it offers
  /// better template deduction compared to the raw constructor invocation.
  ///
  /// \p begin The iterator pointing at the beginning of the sequence.
  /// \p end The iterator pointing at the end of the sequence (one past the last
  ///     segment in the sequence).
  template <typename I>
  static inline GCSegmentRange::Ptr fromConsumable(I begin, I end);

  /// Factory function for creating a range over a single \c AlignedHeapSegment.
  ///
  /// \p seg Pointer to the segment the range will refer to.
  static inline GCSegmentRange::Ptr singleton(AlignedHeapSegment *seg);

  /// Factory function for constructing a concatenation of ranges from its
  /// component parts.
  ///
  /// \p ranges The sequence of component ranges, in the order we wish to
  ///     concatenate them in.
  template <typename... Ranges>
  static inline GCSegmentRange::Ptr concat(std::unique_ptr<Ranges>... ranges);

  virtual ~GCSegmentRange() = default;

  /// Returns a pointer to the next segment, moving the underlying cursor
  /// past the next segment upon success, and returns \c nullptr, leaving the
  /// underlying cursor unchanged, on failure.
  ///
  /// There is no guarantee on the lifetime of the segment pointed to by the
  /// return value, and certain implementations may experience undefined
  /// behaviour if the underlying storage is mutated whilst a range is being
  /// consumed.
  virtual AlignedHeapSegment *next() = 0;

 private:
  template <typename I>
  struct Consumable;

  struct Concat;
};

/// A range backed by a \c ConsumableRange of AlignedHeapSegments.
///
/// \p I A type representing an iterator whose value_type is expected to be
///     AlignedHeapSegment.
template <typename I>
struct GCSegmentRange::Consumable : public GCSegmentRange {
  /// Constructs a new range.
  ///
  /// \p consumable The underlying ConsumableRange of AlignedHeapSegments.
  inline Consumable(ConsumableRange<I> consumable);

  /// See docs for \c GCSegmentRange.
  ///
  /// This range will exhibit undefined behaviour if the iterators backing its
  /// underlying iterator are invalidated.
  inline AlignedHeapSegment *next() override;

 private:
  ConsumableRange<I> consumable_;
};

/// A range composed of the concatenation of a sequence of component ranges.
struct GCSegmentRange::Concat : public GCSegmentRange {
  using Spine = std::vector<GCSegmentRange::Ptr>;

  /// Construct a range from its component ranges.
  ///
  /// \p spine A vector containing the component ranges.
  inline Concat(Spine spine);

  /// See docs for \c GCSegmentRange.
  ///
  /// Let I be the index of the last range we successfully requested a segment
  /// from (initially 0).  The next segment of the concatenation of ranges
  /// R0, R1, ..., RN is the  next segment of the first range in the sequence
  /// RI, R(I + 1), ..., RN to return successfully from a call to \c next, or
  /// \c nullptr if all ranges were unsuccessful in producing a next segment.
  inline AlignedHeapSegment *next() override;

 private:
  Spine ranges_;
  Spine::iterator cursor_;
};

template <typename I>
inline GCSegmentRange::Ptr GCSegmentRange::fromConsumable(I begin, I end) {
  return std::unique_ptr<Consumable<I>>(new Consumable<I>{{begin, end}});
}

inline GCSegmentRange::Ptr GCSegmentRange::singleton(AlignedHeapSegment *seg) {
  return fromConsumable(seg, seg + 1);
}

template <typename... Ranges>
inline GCSegmentRange::Ptr GCSegmentRange::concat(
    std::unique_ptr<Ranges>... ranges) {
  Concat::Spine spine;
  spine.reserve(sizeof...(Ranges));

  // Hack to emit a sequence of emplace_back calls.
  int sink[] = {(spine.emplace_back(std::move(ranges)), 0)...};
  (void)sink;

  return std::unique_ptr<Concat>(new Concat{std::move(spine)});
}

template <typename I>
inline GCSegmentRange::Consumable<I>::Consumable(ConsumableRange<I> consumable)
    : consumable_{std::move(consumable)} {}

template <typename I>
inline AlignedHeapSegment *GCSegmentRange::Consumable<I>::next() {
  return consumable_.hasNext() ? &consumable_.next() : nullptr;
}

inline GCSegmentRange::Concat::Concat(Spine ranges)
    : ranges_{std::move(ranges)}, cursor_{ranges_.begin()} {}

inline AlignedHeapSegment *GCSegmentRange::Concat::next() {
  while (LLVM_LIKELY(cursor_ != ranges_.end())) {
    if (AlignedHeapSegment *res = (*cursor_)->next()) {
      return res;
    } else {
      cursor_++;
    }
  }

  return nullptr;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSEGMENTRANGE_H
