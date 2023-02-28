/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  method(): number {
    return 1;
  }
}

print(new C().method());

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %C(): undefined
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %method(): number
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) "method": string, %1: closure
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: object, %0: closure, "prototype": string
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: closure, "prototype": string
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, %5: any
// CHECK-NEXT:  %7 = LoadParentInst (:object) %6: object
// CHECK-NEXT:  %8 = PrLoadInst (:closure) %7: object, 0: number, "method": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: closure, empty: any, empty: any, %6: object
// CHECK-NEXT:  %10 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, %9: any
// CHECK-NEXT:  %11 = ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:function C(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function method(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1: number
// CHECK-NEXT:function_end