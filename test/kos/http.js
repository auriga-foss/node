'use strict';
require('../common');

const http = require('http');
const port = 8080;

const requestHandler = (request, response) => {
  console.log('Request:' + request.url);
  response.end('Hello World!');
};

const server = http.createServer(requestHandler);
server.listen(port, (err) => {
  if (err) {
    return console.log('Error:', err);
  }
  console.log('opened server on', server.address());
});
