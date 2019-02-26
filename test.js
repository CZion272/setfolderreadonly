
var addon = require('./lib/setfolderreadonly')
var fs = require('fs')

// console.log(addon.openFile("D:\\setfolderreadonly\\123"));
// console.log(addon.openFile("D:\\setfolderreadonly\\123"));
// console.log(addon.closeFile("D:\\setfolderreadonly\\123"));
// // console.log(addon.openWithPrograme("靐龘.exe", "C:\\Users\\Administrator\\Desktop\\501119420.png"));
function run(programe, arg) {
  console.log(addon.openWithPrograme(programe, arg));
  console.log(addon.cpuId());
}

console.log(addon.openFile("D:\\test"));

addon.DiskMessage("D:\\", function(error, freeSpace, totalSpace, freeCanUse)
  {
    console.log(error)
    console.log(freeSpace)
    console.log(totalSpace)
    console.log(freeCanUse)
  }

)