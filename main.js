var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
var binding = require(binding_path);

exports.openFile = function(location) {
  return binding.openFile(location);
}


exports.closeFile = function(location) {
  return binding.closeFile(location);
}

exports.openWithPrograme = function(programe, file) {
  return binding.openWithPrograme(programe, file);
}

exports.findInstall = function(programe) {
  return binding.findInstall(programe);
}