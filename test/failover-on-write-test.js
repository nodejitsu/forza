var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    async = require('async'),
    jsonStream = require('json-stream'),
    cb = require('assert-called');

var PORT0 = 5434,
    PORT1 = 5435,
    gotFirstConnection = false,
    firstClosed = false,
    child;

var server0 = net.createServer(cb(function (socket) {
  var stream = socket.pipe(jsonStream());

  console.log('got first connection');

  gotFirstConnection = true;

  stream.on('readable', cb(function () {
    var chunk = stream.read();
    if (chunk && !firstClosed) {
      console.log('closing first server');
      server0.close();
      socket.destroy();
      firstClosed = true;
    }
  }));
}));

var server1 = net.createServer(cb(function (socket) {
  var stream = socket.pipe(jsonStream());

  console.log('got second connection');
  assert(gotFirstConnection);

  stream.on('readable', cb(function () {
    var chunk = stream.read();
    if (chunk) {
      child.kill();
      process.exit();
    }
  }));
}));

server1.listen(PORT1, cb(function () {
  server0.listen(PORT0, cb(function () {
    child = spawn(
      path.join(__dirname, '..', 'estragon'),
      [
        '-h', '127.0.0.1:' + PORT0.toString(),
        '-h', '127.0.0.1:' + PORT1.toString(),
        '--', 'node', path.join(__dirname, 'fixtures', 'listen.js')
      ]
    );

    setTimeout(function () {
      console.log('timeout');
      child.kill();
      process.exit(1);
    }, 20000);
  }));
}));
