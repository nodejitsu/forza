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
    data = data.split('\n').filter(Boolean).map(JSON.parse).filter(function (d) {
      return d && d.service.indexOf('logs/') === 0;
    });

    var service = { 'logs/stdout': 0, 'logs/stderr': 0 };
    data.forEach(function (d) {
      service[d.service] += d.description.split('\n').filter(Boolean).length
      assert(d.description.match(d.service === 'logs/stdout'
        ? /Hello, stdout!\n/
        : /Hello, stderr!\n/
      ))
    });
    assert.equal(service['logs/stdout'], 1024);
    assert.equal(service['logs/stderr'], 1024);

    server.close();
  });
}));

server.listen(PORT, function () {
  var child = spawn(
    path.join(__dirname, '..', 'estragon'),
    [ '-h', '127.0.0.1:' + PORT.toString(), '--', 'node', path.join(__dirname, 'fixtures', 'output.js') ]
  );
});
