var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    cb = require('assert-called');

var PORT = 5434;

var server = net.createServer(cb(function (socket) {
  var data = '';
  socket.on('readable', cb(function () {
    var chunk = socket.read();

    if (chunk) {
      data += chunk;
    }
  }));

  socket.on('end', cb(function () {
    data = data.split('\n').filter(Boolean).map(JSON.parse);
    assert(data.length > 0);
    server.close();
  }));
}));

server.listen(PORT, function () {
  var child = spawn(
    path.join(__dirname, '..', 'estragon'),
    [
      '-h', '127.0.0.1:' + (PORT - 1).toString(),
      '-h', '127.0.0.1:' + PORT.toString(),
      '--', 'node', path.join(__dirname, 'fixtures', 'exit-after-1-s.js')
    ]
  );
});
