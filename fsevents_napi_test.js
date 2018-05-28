
// const addon = require("./build/Debug/fsevents.node")

const { Constants, FSEvents } = require("./build/Debug/fse.node");

const fse = new FSEvents("./build", function () {
  console.log("in the change");
  console.log(arguments);
});

fse.start();


setTimeout(() => {
  fse.stop();
}, 5000);
