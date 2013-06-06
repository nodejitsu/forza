var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    jsonStream = require('json-stream'),
    cb = require('assert-called');

var PORT = 5434;

var server = net.createServer(cb(function (socket) {
  var stream = socket.pipe(jsonStream()),
      chunks = [];

  stream.on('readable', cb(function () {
    var chunk = stream.read();
    if (chunk) {
      chunks.push(chunk);
    }
  }));

  socket.on('end', cb(function () {
    assert(chunks.length > 0);
    server.close();
  }));
}));

server.listen(PORT, function () {
  var child = spawn(
    path.join(__dirname, '..', 'estragon'),
    [
      '-h', '127.0.0.1:' + (PORT - 2).toString(),
      '-h', '127.0.0.1:' + (PORT - 1).toString(),
      '-h', '127.0.0.1:' + PORT.toString(),
      '--', 'node', path.join(__dirname, 'fixtures', 'output.js')
    ]
  );
});
