var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    jsonStream = require('json-stream'),
    cb = require('assert-called');

var PORT = 5437,
    child,
    gotMessage = false;

var server = net.createServer(cb(function (socket) {
  var stream = socket.pipe(jsonStream());

  stream.on('readable', cb(function () {
    var chunk = stream.read(),
        clientPort;

    if (chunk && chunk.service === 'health/process/start') {
      assert.equal(chunk.metric, 1.0);
      gotMessage = true;

      child.kill();
      socket.destroy();
      server.close();
      process.exit();
    }
  }));
}));

server.listen(PORT, cb(function () {
  child = spawn(
    path.join(__dirname, '..', 'estragon'),
    [ '-h', '127.0.0.1:' + PORT.toString(), '--', 'node', path.join(__dirname, 'fixtures', 'listen.js') ]
  );
}));

process.on('exit', function () {
  assert(gotMessage);
});
