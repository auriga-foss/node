'use strict'
const common = require('../common');
// Flags: --inspect --expose-internals
common.skipIfInspectorDisabled();

delete process.config;
process.config = {};
