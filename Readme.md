# node-archive [![Build Status](https://secure.travis-ci.org/Skomski/node-archive.png?branch=master)](http://travis-ci.org/Skomski/node-archive)

Node.js bindings for libarchive (https://github.com/libarchive/libarchive)

## Install

```
npm install archive
```

## Usage

```javascript

var Archive = require('archive');
var buffer  = new Buffer('yyyyyyyyyyyyyyyyyyy');

Archive.compress(buffer, function(err, compressed){
  if (err) throw err;

  Archive.decompress(compressed, function(err, decompressed){
    if (err) throw err;
  });
});
```

## Methods

### compress(buffer, cb)
  * Required:
    * `buffer` - Raw buffer
    * `cb` - Function with two arguments `(err, compressedBuffer)`

### decompress(buffer, cb)
  * Required:
    * `buffer` - Compressed buffer
    * `cb` - Function with two arguments `(err, decompressedBuffer)`

## License

Licensed under the MIT license.
