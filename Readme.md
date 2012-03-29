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

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright interest in the software to the public domain. We make this dedication for the benefit of the public at large and to the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity of all present and future rights to this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to http://unlicense.org/
