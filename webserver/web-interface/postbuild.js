const fs = require('fs');
const path = require('path');
// Assets folder regex
const filenameRegex = /^index-[a-zA-Z0-9]+\.js$/;

const deleteData = (filePath) => {
  if (fs.existsSync(filePath)) {
    if (fs.lstatSync(filePath).isDirectory()) {
      fs.readdirSync(filePath).forEach((file) => {
        const fullPath = path.join(filePath, file);
        deleteData(fullPath);
      });
      fs.rmdirSync(filePath);
    } else {
      fs.unlinkSync(filePath);
    }
  }
};

const matchesRegex = (filePath) => filenameRegex.test(path.basename(filePath));
const deleteAssetsMatchingRegex = (dirPath) => {
  if (fs.existsSync(dirPath)) {
    fs.readdirSync(dirPath).forEach((file) => {
      const fullPath = path.join(dirPath, file);
      if (matchesRegex(fullPath)) {
        deleteData(fullPath);
      }
    });
  }
};


deleteAssetsMatchingRegex('../data/assets');
deleteData('../data/index.html');

console.log('Files deleted successfully.');
