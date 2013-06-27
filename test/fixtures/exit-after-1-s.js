setTimeout(function () {
  console.log('100 ms');
}, 100);

setTimeout(function () {
  console.log('500 ms');
}, 500);

setTimeout(function () {
  process.exit(0);
}, 1000);
