'use strict';

const common = require('../common');

// Flags: --inspect --expose-internals
common.skipIfInspectorDisabled();

const fixtures = require('../common/fixtures');
const startCLI = require('../common/debugger');

// Test for "Breakpoint at specified location already exists" error.
const script = fixtures.path('debugger', 'three-lines.js');
const cli = startCLI([script]);

(async () => {
  try {
    await cli.waitForInitialBreak();
    await cli.waitForPrompt();
    await cli.command('setBreakpoint(1)');
    await cli.command('setBreakpoint(1)');
    await cli.waitFor(/Breakpoint at specified location already exists/);
  } finally {
    await cli.quit();
  }
})().then(common.mustCall());
