var Archive = require('..');
var Assert = require('assert');
var Fs     = require('fs');

var tests = 0;

test = function(name, file) {
  Archive.decompress(file, __dirname + '/result/', function(err, decompressed){
    Assert.ifError(err);
    tests++;
  });
}

test('file', __dirname + '/archive.tar');

process.on('exit', function() {
  Assert.equal(tests, 1);
});
