var snappy;

try {
  snappy = require('../build/default/binding');
} catch(e) {
  snappy = require('../build/Release/binding');
}

exports.compress = function(buffer, cb) {
  if(!Buffer.isBuffer(buffer)) {
    return cb(new Error('You need to specify a buffer!'));
  }
  snappy.compress(buffer, cb);
};

exports.decompress = function(buffer, cb) {
  if(!Buffer.isBuffer(buffer)) {
    return cb(new Error('You need to specify a buffer!'));
  }
  snappy.uncompress(buffer, cb);
};
