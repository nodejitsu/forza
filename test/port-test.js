var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    cb = require('assert-called');

var PORT = 5433;

var server = net.createServer(cb(function (socket) {
  var data = '';

  socket.on('readable', cb(function () {
    var clientPort;

    // FIXME assumes that one message comes in one chunk but 4 AM
    var chunk = socket.read();
    if (!chunk) {
      return;
    }

    chunk = JSON.parse(chunk);

    if (chunk.service === 'port') {
      clientPort = parseInt(chunk.description, 10);
      console.log('client port', clientPort);
      clientSocket = net.connect(clientPort, cb(function () {
        var data = '';

        console.log('connected to client');

        clientSocket.on('readable', cb(function () {
          var chunk = clientSocket.read();
          if (!chunk) {
            return;
          }
          data += chunk;
        }));

        clientSocket.on('end', cb(function () {
          assert.equal(data, 'pong\n');
          clientSocket.end();
          console.log('got pong');
        }));

        clientSocket.write('ping\n');
      }));
    }
  }));

  socket.on('end', function () {
    server.close();
  });
}));

server.listen(PORT, cb(function () {
  var child = spawn(
    path.join(__dirname, '..', 'estragon'),
    [ '-h', '127.0.0.1', '-p', PORT.toString(), '--', 'node', path.join(__dirname, 'fixtures', 'listen.js') ]
  );
}));
