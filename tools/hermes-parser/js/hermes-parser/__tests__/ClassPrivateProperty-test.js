/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {AlignmentCase} from '../__test_utils__/alignment-utils';

import {
  expectBabelAlignment,
  expectEspreeAlignment,
} from '../__test_utils__/alignment-utils';
import {parse, parseForSnapshot} from '../__test_utils__/parse';

const PRIVATE_ERROR_MESSAGE = 'Private properties are not supported';
describe('Private properties', () => {
  describe('Property Definition', () => {
    const testCase: AlignmentCase = {
      code: `
        class Foo {
          #private;
        }
      `,
      espree: {
        expectToFail: false,
      },
      babel: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "body": Object {
                "body": Array [
                  Object {
                    "computed": false,
                    "declare": false,
                    "key": Object {
                      "name": "private",
                      "type": "PrivateIdentifier",
                    },
                    "optional": false,
                    "static": false,
                    "type": "PropertyDefinition",
                    "typeAnnotation": null,
                    "value": null,
                    "variance": null,
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": Array [],
              "id": Object {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": Array [],
              "superClass": null,
              "superTypeParameters": null,
              "type": "ClassDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Private property uses are not supported
      expect(() => parse(testCase.code, {babel: true})).toThrow(
        new SyntaxError('Private properties are not supported (3:10)'),
      );
      expectBabelAlignment(testCase);
    });
  });

  describe('Member Expression', () => {
    const testCase: AlignmentCase = {
      code: `
        class Foo {
          #private;
          constructor() {
            foo.#private;
          }
        }
      `,
      espree: {
        expectToFail: false,
      },
      babel: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "body": Object {
                "body": Array [
                  Object {
                    "computed": false,
                    "declare": false,
                    "key": Object {
                      "name": "private",
                      "type": "PrivateIdentifier",
                    },
                    "optional": false,
                    "static": false,
                    "type": "PropertyDefinition",
                    "typeAnnotation": null,
                    "value": null,
                    "variance": null,
                  },
                  Object {
                    "computed": false,
                    "key": Object {
                      "name": "constructor",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "constructor",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": Object {
                      "async": false,
                      "body": Object {
                        "body": Array [
                          Object {
                            "directive": null,
                            "expression": Object {
                              "computed": false,
                              "object": Object {
                                "name": "foo",
                                "optional": false,
                                "type": "Identifier",
                                "typeAnnotation": null,
                              },
                              "optional": false,
                              "property": Object {
                                "name": "private",
                                "type": "PrivateIdentifier",
                              },
                              "type": "MemberExpression",
                            },
                            "type": "ExpressionStatement",
                          },
                        ],
                        "type": "BlockStatement",
                      },
                      "expression": false,
                      "generator": false,
                      "id": null,
                      "params": Array [],
                      "predicate": null,
                      "returnType": null,
                      "type": "FunctionExpression",
                      "typeParameters": null,
                    },
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": Array [],
              "id": Object {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": Array [],
              "superClass": null,
              "superTypeParameters": null,
              "type": "ClassDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Private property uses are not supported
      expect(() => parse(testCase.code, {babel: true})).toThrow(
        new SyntaxError('Private properties are not supported (3:10)'),
      );
      expectBabelAlignment(testCase);
    });
  });

  describe('Brand Check', () => {
    const testCase: AlignmentCase = {
      code: `
        class Foo {
          #private;
          constructor() {
            #private in foo;
          }
        }
      `,
      espree: {
        expectToFail: false,
      },
      babel: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "body": Object {
                "body": Array [
                  Object {
                    "computed": false,
                    "declare": false,
                    "key": Object {
                      "name": "private",
                      "type": "PrivateIdentifier",
                    },
                    "optional": false,
                    "static": false,
                    "type": "PropertyDefinition",
                    "typeAnnotation": null,
                    "value": null,
                    "variance": null,
                  },
                  Object {
                    "computed": false,
                    "key": Object {
                      "name": "constructor",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "kind": "constructor",
                    "static": false,
                    "type": "MethodDefinition",
                    "value": Object {
                      "async": false,
                      "body": Object {
                        "body": Array [
                          Object {
                            "directive": null,
                            "expression": Object {
                              "left": Object {
                                "name": "private",
                                "type": "PrivateIdentifier",
                              },
                              "operator": "in",
                              "right": Object {
                                "name": "foo",
                                "optional": false,
                                "type": "Identifier",
                                "typeAnnotation": null,
                              },
                              "type": "BinaryExpression",
                            },
                            "type": "ExpressionStatement",
                          },
                        ],
                        "type": "BlockStatement",
                      },
                      "expression": false,
                      "generator": false,
                      "id": null,
                      "params": Array [],
                      "predicate": null,
                      "returnType": null,
                      "type": "FunctionExpression",
                      "typeParameters": null,
                    },
                  },
                ],
                "type": "ClassBody",
              },
              "decorators": Array [],
              "id": Object {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "implements": Array [],
              "superClass": null,
              "superTypeParameters": null,
              "type": "ClassDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Private property uses are not supported
      expect(() => parse(testCase.code, {babel: true})).toThrow(
        new SyntaxError('Private properties are not supported (3:10)'),
      );
      expectBabelAlignment(testCase);
    });
  });
});
