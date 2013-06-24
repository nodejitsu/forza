var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    jsonStream = require('json-stream'),
    cb = require('assert-called');

var PORT = 5437,
    gotMessage = false;

var server = net.createServer(cb(function (socket) {
  var stream = socket.pipe(jsonStream());

  stream.on('readable', cb(function () {
    var chunk = socket.read(),
        clientPort;

    if (chunk && chunk.service === 'health/process/start') {
      assert.equal(chunk.metric, 1.0);
      gotMessage = true;
    }
  }));

  socket.on('end', function () {
    server.close();
  });
}));

server.listen(PORT, cb(function () {
  var child = spawn(
    path.join(__dirname, '..', 'estragon'),
    [ '-h', '127.0.0.1:' + PORT.toString(), '--', 'node', path.join(__dirname, 'fixtures', 'listen.js') ]
  );
}));

process.on('exit', function () {
  assert(gotMessage);
});
