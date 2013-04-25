var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    cb = require('assert-called');

var PORT = 5432;

var server = net.createServer(cb(function (socket) {
  var data = '';

  socket.on('readable', function () {
    var chunk = socket.read();
    data += chunk;
  });

  socket.on('end', function () {
    data = data.split('\n').filter(Boolean).map(JSON.parse);

    assert.equal(data[0].service, 'stdout');
    assert.equal(data[0].description, 'Hello, stdout!\n');

    assert.equal(data[1].service, 'stderr');
    assert.equal(data[1].description, 'Hello, stderr!\n');

    server.close();
  });
}));

server.listen(PORT, function () {
  var child = spawn(
    path.join(__dirname, '..', 'estragon'),
    [ '-h', '127.0.0.1', '-p', PORT.toString(), '--', 'node', path.join(__dirname, 'fixtures', 'output.js') ]
  );
});
