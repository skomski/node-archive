var archive;

try {
  archive = require('../build/default/binding');
} catch(e) {
  console.error(e);
  archive = require('../build/Release/binding');
}

exports.decompress = function(input, output, cb) {
  if(typeof input != 'string') {
    return cb(new Error('You need to specify a input path!'));
  }
  if(typeof output != 'string') {
    return cb(new Error('You need to specify a output path!'));
  }
  archive.decompress(input, output, cb);
};
