var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
var addon = require(binding_path);

// console.log(addon.openFile("D:\\setfolderreadonly\\123"));
// console.log(addon.openFile("D:\\setfolderreadonly\\123"));
// console.log(addon.closeFile("D:\\setfolderreadonly\\123"));

console.log(addon.openWithPrograme("C:\\Users\\Administrator\\Desktop\\DKH-ZHJ--机构客户服务.doc", 1));
function run(programe, arg) {
  console.log(addon.openWithPrograme(programe, arg));
  console.log(addon.cpuId());
}

console.log(addon.openFile("D:\\test"));

addon.DiskMessage("D:\\", function(error, freeSpace, totalSpace, freeCanUse)
  {
    console.log(123)
    console.log(error)
    console.log(freeSpace)
    console.log(totalSpace)
    console.log(freeCanUse)
  }

)
console.log(123);
addon.copyFile("C:\\Users\\Administrator\\Desktop\\322-0.png*C:\\Users\\Administrator\\Desktop\\59a3e0ab68322.png");