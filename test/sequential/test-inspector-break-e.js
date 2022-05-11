'use strict';
const common = require('../common');
// Flags: --inspect --expose-internals
common.skipIfInspectorDisabled();

const assert = require('assert');
const { NodeInstance } = require('../common/inspector-helper.js');

async function runTests() {
  const instance = new NodeInstance(undefined, 'console.log(10)');
  const session = await instance.connectInspectorSession();
  await session.send([
    { 'method': 'Runtime.enable' },
    { 'method': 'Debugger.enable' },
    { 'method': 'Runtime.runIfWaitingForDebugger' },
  ]);
  await session.waitForBreakOnLine(0, '[eval]');
  await session.runToCompletion();
  assert.strictEqual((await instance.expectShutdown()).exitCode, 0);
}

runTests().then(common.mustCall());
