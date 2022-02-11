'use strict';
require('../common');

const Os = require('os');

console.log('PID           : ' + process.pid);
console.log('OS            : ' + Os.platform() + ' ' + Os.release());
console.log('Date          : ' + Date());
console.log('Directory     : ' + process.cwd());

const memory = process.memoryUsage();
console.log('Memory        : ' + memory.heapUsed + ' / ' + memory.heapTotal);

const ni = Os.networkInterfaces();
console.log('Network       : ' + Object.keys(ni).length + ' interfaces');
for (const i in ni) {
  let iface = i + ':';
  for (const j in ni[i]) iface += ' ' + ni[i][j].address;
  console.log(iface);
}
