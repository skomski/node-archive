# node-archive [![Build Status](https://secure.travis-ci.org/Skomski/node-archive.png?branch=master)](http://travis-ci.org/Skomski/node-archive)

Node.js bindings for libarchive (https://github.com/libarchive/libarchive)

## WIP

* ArchiveWriter

## Install

```
npm install archive
```

## Reader

```javascript

var Archive = require('archive');

var archive = new Archive.Reader({
  filename: 'snappy.tar.gz'
});

archive.on('file', function(file) {
  console.log(entry.path, entry.mtime);

  entry.on('data', function(buffer) {
    console.log(buffer.toString());
    file.nextChunk();
  });

  entry.on('end', function(buffer) {
    console.log('end');
    reader.nextChunk();
  });
  file.nextChunk();
});

archive.on('error', function(err) {
  console.error(err);
});

archive.open(function() {
  archive.nextEntry();
});
```

## License

Licensed under the MIT license.
