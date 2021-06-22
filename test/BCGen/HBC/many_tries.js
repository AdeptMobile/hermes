/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -dump-bytecode -O %s | %FileCheck --match-full-lines %s

// This test just checks that we can compile this code without failing, since a
// previous bug caused this to trigger recursive behaviour in the compiler.

try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}
try{}catch(a){}

//CHECK-LABEL:Function<global>(1 params, {{[0-9]+}} registers, 0 symbols):
