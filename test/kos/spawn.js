console.log('[TEST_SPAWN_LOG]: Check child_process module');
const { spawn } = require('child_process');

console.log('[TEST_SPAWN_LOG]: Attempt to create child object be KasperskyOS-specific spawn() from ramfs');
const childRamFs = spawn('/application',  ["ARG1", "ARG2", "ARG3"]);
console.log('[TEST_SPAWN_LOG]: Success')

// console.log('[TEST_SPAWN_LOG]: Attempt to create child object be KasperskyOS-specific spawn() from romfs');
// const childRomFs = spawn('/romfs/application',  ["ARG1", "ARG2", "ARG3"]);
// console.log('[TEST_SPAWN_LOG]: Success')

childRamFs.stdout.on('data', (data) => {
  console.log(`child stdout:\n${data}`);
  });

childRamFs.stderr.on('data', (data) => {
  console.error(`child stderr:\n${data}`);
  });

childRamFs.on('exit', function (code, signal) {
    console.log('child process exited with ' +
                `code ${code} and signal ${signal}`);
  });

// childRomFs.stdout.on('data', (data) => {
//   console.log(`child stdout:\n${data}`);
//   });

// childRomFs.stderr.on('data', (data) => {
//   console.error(`child stderr:\n${data}`);
//   });

// childRomFs.on('exit', function (code, signal) {
//     console.log('child process exited with ' +
//                 `code ${code} and signal ${signal}`);
//   });