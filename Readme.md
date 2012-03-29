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

var reader = new Archive.Reader({
  path: __dirname + '/snappy.tar.gz' // specify source path
});

reader.on('directory', function(directory) {
  console.log(directory.path);
  reader.nextEntry();
});

reader.on('file', function(file) {
  console.log(file.path, file.mtime);
  
  file.on('error', function(err) {
    console.error(err);
  });

  file.on('data', function(buffer) {
    console.log(buffer.toString());
    reader.nextChunk(); // get next chunk
  });

  file.on('end', function() {
    console.log('file end');
    reader.nextEntry(); // get next entry
  });
  
  reader.nextChunk(); // get first chunk
});

reader.on('error', function(err) { // archive error
  console.error(err);
});

reader.on('end', function() {
  console.log('archive end');
});

reader.open(function(info) { // open archive
  console.log(info.compression);
  reader.nextEntry();  // get first entry
});
```

## License

Licensed under the MIT license.
