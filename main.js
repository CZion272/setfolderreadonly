var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
var binding = require(binding_path);

var addon = require('./lib/setFolderReadOnly')

exports.setFolderReadOnly = function(location) {
  return addon.setFolderReadOnly(location);
}
