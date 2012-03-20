# node-archive [![Build Status](https://secure.travis-ci.org/Skomski/node-archive.png?branch=master)](http://travis-ci.org/Skomski/node-archive)

Node.js bindings for libarchive (https://github.com/libarchive/libarchive)

## WIP

* ArchiveWriter

## Dependencies

```
libarchive
http://libarchive.github.com/
>= 3.0.3
````

## Install

```
npm install archive
```

## Reader

```javascript

var Archive = require('archive');

var archive = new Archive.Reader({
  path: 'snappy.tar.gz'
});

archive.on('directory', function(directory) {
  console.log(directory.path);
  reader.nextEntry();
});

archive.on('file', function(file) {
  console.log(entry.path, entry.mtime);
  
  file.on('error', function(err) {
    console.error(err);
  });

  file.on('data', function(buffer) {
    console.log(buffer.toString());
    file.nextChunk();
  });

  file.on('end', function(buffer) {
    console.log('end');
    reader.nextEntry();
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
