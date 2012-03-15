# node-archive [![Build Status](https://secure.travis-ci.org/Skomski/node-archive.png?branch=master)](http://travis-ci.org/Skomski/node-archive)

Node.js bindings for libarchive (https://github.com/libarchive/libarchive)

## WIP

* Node.js Event API

## Install

```
npm install archive
```

## Usage

```javascript

var Archive = require('archive');

Archive.decompress(__dirname + '/archive.tar.gz', __dirname, function(err, decompressed){
  if (err) throw err;
});
```

## Methods

### decompress(buffer, cb)
  * Required:
    * `input`  - Path to source archive
    * `output` - Path to target directory
    * `cb` - Function with two arguments `(err, files)`

## License

Licensed under the MIT license.
