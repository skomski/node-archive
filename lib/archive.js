var Events = require('events');
var Util   = require('util');
var Archive;

function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}

try {
  Archive = require('../build/default/addon');
} catch(e) {
  Archive = require('../build/Release/addon');
}

inherits(Archive.Reader, Events.EventEmitter);
inherits(Archive.Entry, Events.EventEmitter);

exports.ENTRY_TYPES = {
  FILE: 32768,
  DIRECTORY: 16384
}

exports.createFileReader = function(options) {
  if (!options.filename) return new Error('You need to provide a filename!');

  var reader = new Archive.Reader(options.filename);
  reader.on('entry', function(entry) {
    switch (entry.type) {
      case exports.ENTRY_TYPES.DIRECTORY:
        reader.emit('directory', entry);
        break;
      case exports.ENTRY_TYPES.FILE:
        reader.emit('file', entry);
        break;
    }
  });
  return reader;
}
