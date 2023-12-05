/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import * as path from 'path';
import {createModuleGraph} from './ModuleGraph';
import {parseFile, hermesASTToBabel, writeBundle} from './utils';
import yargs from 'yargs';

// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import generate from '@babel/generator';
// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import {transformFromAstSync} from '@babel/core';

async function main() {
  const rawArgs = process.argv.slice(2);
  const cliYargs = yargs(rawArgs)
    .usage('Usage: $0 [entrypoint]')
    .scriptName('flow-bundler')
    .option('out', {
      alias: 'o',
      describe: 'Path to output the bundle',
      nargs: 1,
      required: true,
      requiresArg: true,
      type: 'string',
    })
    .option('es5', {
      describe:
        'Create ES5 syntax compatible bundle along side standard bundle',
      default: false,
      type: 'boolean',
    })
    .option('root', {
      alias: 'r',
      describe:
        'Root path, paths will be shown relative to this location (default: cwd).',
      nargs: 1,
      requiresArg: true,
      type: 'string',
      default: process.cwd(),
    })
    .positional('entrypoint', {
      type: 'string',
      describe: 'Bundle entrypoint files',
    })
    .wrap(yargs.terminalWidth())
    .parse();

  const rootPath = cliYargs.root;
  const outPath = cliYargs.out;
  const createES5Bundle = cliYargs.es5;
  const entrypoints: Array<string> = cliYargs._;

  const bundle = await createModuleGraph(rootPath, entrypoints);
  const bundleHeader = `*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * ${'@'}flow
 * ${'@'}generated
 *
 * Entrypoints:
 *   ${entrypoints.map(f => path.relative(rootPath, f)).join('\n *   ')}
 `;

  const fileMapping: {[string]: string} = {};
  const bundleAST = {
    type: 'File',
    program: {
      type: 'Program',
      // $FlowExpectedError[unclear-type]
      body: [] as Array<any>,
      directives: [],
    },
  };

  // Merge files into single bundle AST.
  for (const file of bundle) {
    const babelAST = hermesASTToBabel(file.ast, file.file);

    bundleAST.program.body.push(...babelAST.program.body);
    fileMapping[file.file] = file.code;
  }

  // Add bundle docblock comment
  if (bundleAST.program.body.length > 0) {
    const firstStmt = bundleAST.program.body[0];
    firstStmt.leadingComments = [
      {type: 'CommentBlock', value: bundleHeader},
      ...firstStmt.leadingComments,
    ];
  }

  // Generate bundle for Static Hermes
  const shBundleSource = generate(bundleAST, {sourceMaps: true}, fileMapping);
  await writeBundle(outPath, shBundleSource.code, shBundleSource.map);

  if (createES5Bundle) {
    // Generate bundle for ES5 runtimes (aka normal Hermes)
    const es5Bundle = transformFromAstSync(bundleAST, fileMapping, {
      ast: true,
      code: false,
      presets: [
        [
          require.resolve('metro-react-native-babel-preset'),
          {enableBabelRuntime: false},
        ],
      ],
    });
    const es5bundleSource = generate(
      es5Bundle.ast,
      {sourceMaps: true},
      fileMapping,
    );
    await writeBundle(
      outPath.replace('.js', '-es5.js'),
      es5bundleSource.code,
      es5bundleSource.map,
    );
  }
}

main().catch((err: mixed) => {
  console.error(err);
  process.exitCode = 1;
});