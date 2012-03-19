var Archive = require('..');
var Assert = require('assert');
var Fs     = require('fs');
var spawn = require('child_process').spawn;

var tests = 0;

diff = function(dir, cb) {
  var proc = spawn('diff', [__dirname + '/result/' + dir, __dirname + '/fixture-diff/' + dir]);
  proc.stderr.on('data', function(data) {
    console.error(data.toString());
  });
  proc.stdout.on('data', function(data) {
    console.log(data.toString());
  });
  proc.on('exit', function(code) {
    if (code != 0) throw new Error('DiffError: ' + code);
    cb();
  });
}

test = function(file, dir) {
  var reader = Archive.createFileReader({
    filename: file
  });

  reader.on('directory', function(directory) {
    try {
      Fs.mkdirSync(__dirname + '/result/' + directory.path);
    } catch (error) {
    }
    reader.nextEntry();
  });

  reader.on('file', function(file) {
    var stream = Fs.createWriteStream(__dirname + '/result/' + file.path);

    file.on('data', function(chunk) {
      stream.write(chunk);
      file.nextChunk();
    });

    file.on('end', function() {
      stream.end();
      reader.nextEntry();
    });

    file.nextChunk();
  });

  reader.on('error', function(err) {
    if (err) throw err;
  });

  reader.on('end', function() {
    diff(dir, function() {
      tests++;
    })
  });

  reader.open(function(archive) {
    reader.nextEntry();
  });
}

test(__dirname + '/fixture/snappy-1.0.5.tar.gz', 'snappy-1.0.5');
//test('file', __dirname + '/fixture/up.zip');
//test('file', __dirname + '/fixture/node.pkg');
//test('file', __dirname + '/fixture/theme.rar');

process.on('exit', function() {
  Assert.equal(tests, 1);
});

