var Archive = require('..');
var Assert = require('assert');
var Fs     = require('fs');
var spawn = require('child_process').spawn;
var rimraf = require('rimraf');

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

test = function(file, dir, compression) {
  var reader = Archive.createFileReader({
    path: file
  });

  reader.on('directory', function(directory) {
    Fs.mkdir(__dirname + '/result/' + directory.path, 0777, function(err) {
      if (err) throw err;
      reader.nextEntry();
    });
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
    Assert.equal(archive.compression, compression);
    reader.nextEntry();
  });
}

rimraf(__dirname + '/result', function() {
  Fs.mkdir(__dirname + '/result', 0777, function(err) {
    if (err) throw err;
    test(__dirname + '/fixture/snappy-1.0.5.tar.gz', 'snappy-1.0.5', 'gzip');
    test(__dirname + '/fixture/archive.xz', 'archive', 'xz');
    test(__dirname + '/fixture/glyphicons_free.zip', 'glyphicons_free', 'none');
  });
});

process.on('exit', function() {
  Assert.equal(tests, 3);
});

