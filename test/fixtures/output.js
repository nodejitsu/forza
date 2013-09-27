for (var i = 0; i < 1024; i++) {
  process.stdout.write('Hello, \x1B[32mstdout\x1B[39m!\n');
  process.stderr.write('Hello, \x1B[31mstderr\x1B[39m!\n');
}
