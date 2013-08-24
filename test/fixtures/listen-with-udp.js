var net = require('net'),
    dgram = require('dgram');

dgram.createSocket("udp4").bind(5340, function () {
  net.createServer().listen(5341);
});
