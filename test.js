
var addon = require('./lib/setfolderreadonly')
var fs = require('fs')

// console.log(addon.openFile("D:\\setfolderreadonly\\123"));
// console.log(addon.openFile("D:\\setfolderreadonly\\123"));
// console.log(addon.closeFile("D:\\setfolderreadonly\\123"));

console.log(addon.openWithPrograme("C:\\Users\\Administrator\\Desktop\\322-0.png", 1));
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
// addon.copyFile("C:\\Users\\Administrator\\Desktop\\322-0.png*C:\\Users\\Administrator\\Desktop\\59a3e0ab68322.png");