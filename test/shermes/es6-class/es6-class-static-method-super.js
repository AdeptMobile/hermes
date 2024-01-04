/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xes6-class -exec -Wx,-Xeval-es6-class %s | %FileCheck --match-full-lines %s

class Parent {
  static whoAmI() {
    return "Parent";
  }
}

class Child extends Parent {
  static whoAmI() {
    return `I'm the Child of ${super.whoAmI()}`;
  }
}

print(Parent.whoAmI());
//CHECK: Parent

print(Child.whoAmI());
//CHECK: I'm the Child of Parent
