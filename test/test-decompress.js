var Snappy = require('..');
var Assert = require('assert');
var Fs     = require('fs');

var tests = 0;

test = function(name, buffer) {
  Snappy.decompress(buffer, function(err, decompressed){
    Assert.ifError(err);
    Assert.equal(decompressed.toString(), buffer.toString());
    console.log('ok ' + ++tests + ' ' + name);
  });
}

test('raw', new Buffer([21, 'ef', 'bf', 'bd', '7b', 22, 75, 75, 69, 64, 22, '3a', 22, 64, 37, 35, 38, 62, 65, 36, 33, '2d', 33, 66, 38, 22, '2c', 22, 64, 61, 74, 61, 22, '3a', '7b', '7d', '7d']));

process.on('exit', function() {
  Assert.equal(tests, 1);
});
