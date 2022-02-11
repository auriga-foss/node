'use strict';
require('../common');

// The '.node' suffix will make nodejs process addon as a dynamic executable
// file.
const addon = require('/test/kos/addons/test_addon.node');

// File has to presist in the FS (even empty? so far we put '1' into the file)
// and has to be 'discoverable by nodejs test file name code flow.

// eventually, say hello from addon
console.log(addon.hello());
