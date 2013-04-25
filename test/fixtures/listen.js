var net = require('net');

var server = net.createServer(function (socket) {
  var data = '';

  socket.on('readable', function () {
    data += socket.read();

    if (data === 'ping\n') {
      socket.write('pong\n');
      socket.end();
      server.close();
    }
  });
}).listen(5333);
