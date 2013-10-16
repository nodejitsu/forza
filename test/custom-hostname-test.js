var net = require('net'),
    path = require('path'),
    assert = require('assert'),
    spawn = require('child_process').spawn,
    jsonStream = require('json-stream'),
    cb = require('assert-called');

var got = false,
    PORT = 5435,
    HOSTNAME = 'forza-test',
    child;

process.on('exit', function () {
  assert(got);
});

var server = net.createServer(cb(function (socket) {
  var stream = socket.pipe(jsonStream());

  stream.on('readable', cb(function () {
    var chunk = stream.read();

    if (chunk) {
      got = true;
      child.kill();
      assert.equal(chunk.host, HOSTNAME);
      process.exit();
    }
  }));
}));

server.listen(PORT, function () {
  child = spawn(
    path.join(__dirname, '..', 'forza'),
    [ 
      '-h', '127.0.0.1',
      '-p',  PORT.toString(),
      '--hostname', HOSTNAME,
      '--', 'node', path.join(__dirname, 'fixtures', 'exit-after-1-s.js')
    ]
  );
});
