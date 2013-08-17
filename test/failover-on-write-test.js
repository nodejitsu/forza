var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    async = require('async'),
    jsonStream = require('json-stream'),
    cb = require('assert-called');

var PORT0 = 5434,
    PORT1 = 5435,
    connections = 0,
    firstClosed = false,
    child;

function onConnection(socket) {
  var stream = socket.pipe(jsonStream()),
      server = this;

  console.log('got connection', this._name);
  ++connections;
  console.log(connections);

  stream.on('readable', cb(function () {
    var chunk = stream.read();
    if (!chunk) {
      return;
    }
    console.dir(chunk);

    if (connections === 1 && !firstClosed) {
      console.log('first closed');
      server.close();
      socket.destroy();
      firstClosed = true;
    }
    else if (connections >= 2) { // For some reason, `connections === 2` wouldn't work
      console.log('complete');
      server.close();
      socket.destroy();
      child.kill();
      process.exit();
    }
  }));
}

var server0 = net.createServer(cb(onConnection)),
    server1 = net.createServer(cb(onConnection));

server1.listen(PORT1, cb(function () {
  server0.listen(PORT0, cb(function () {
    child = spawn(
      path.join(__dirname, '..', 'forza'),
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
