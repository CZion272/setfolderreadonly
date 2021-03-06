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

exports.openWithPrograme = function(file, mod) {
  return binding.openWithPrograme(file, mod);
}

exports.findInstall = function(programe) {
  return binding.findInstall(programe);
}

exports.DiskMessage = function(path, func) {
  return binding.DiskMessage(path, func);
}

//Use '*' to parting path
exports.copyFile = function(path) {
  return binding.copyFile(path);
}