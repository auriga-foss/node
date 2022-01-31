'use strict';
require('../common');

// Interpreter/JIT load. '_rawDebug\ can be removed in case of stdout/stderr bugs.

function calculatePI(iterations = 10000) {
  // https://stackoverflow.com/questions/30747235/javascript-pi-%CF%80-calculator
  let pi = 0;
  const iterator = sequence();

  for (let i = 0; i < iterations; i++) {
    pi += 4 / iterator.next().value;
    pi -= 4 / iterator.next().value;
  }

  function* sequence() {
    let i = 1;
    while (true) {
      yield i;
      i += 2;
    }
  }

  return pi;
}

for (let i = 25; i > 0; i--) {
  process._rawDebug(calculatePI());
}
