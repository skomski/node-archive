var Snappy = require('..');
var Assert = require('assert');
var Fs     = require('fs');

var tests = 0;

test = function(name, buffer) {
  Snappy.compress(buffer, function(err, compressed){
    Assert.ifError(err);

    Snappy.decompress(compressed, function(err, decompressed){
      Assert.ifError(err);
      Assert.equal(decompressed.toString(), buffer.toString());
      console.log('ok ' + ++tests + ' ' + name);
    });
  });
}

test('simple', new Buffer('spppppppppassssssssssssssspuuuuuuuuuuur!!___"DSA'));
test('raw', new Buffer([255, 200, 100, 3, 0, 256, 80]));
test('utf8', new Buffer('काक्नोम्यत्क्नोम्यत्चं शक्नोम्यत्तुमतुम् ।तुम् ।् । नोपहिनस्ति माम् ॥'));
test('json', new Buffer(JSON.stringify({
  status: 404,
  errorName: 'NoSuchBucket',
  errorMessage: 'The specified bucket does not exist.',
  uuid: '550e8400-e29b-41d4-a716-4466554400002'
})));
test('json2', new Buffer(JSON.stringify({
  uuid: 'd758be63-3f8',
  data: {
    'ddddddddd': 'yyyyyyyyyyyyyyyyyyyy'
  }
})));
test('file', Fs.readFileSync(__dirname + '/urls.10K'));

process.on('exit', function() {
  Assert.equal(tests, 6);
});
