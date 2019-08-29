const { XScreencap } = require('./build/Release/xscreencap');
const { PNG } = require('pngjs');
const fs = require('fs');

const SegfaultHandler = require('segfault-handler');

SegfaultHandler.registerHandler("crash.log");

let xsc = new XScreencap();

console.log(xsc.connect());

// let image = xsc.getImage(-1);

xsc.getImageAsync(-1, (image) => {
    console.log(image);

    if (image.error == null) {
        let png = new PNG({
            width: image.width,
            height: image.height
        });
    
        for(let i = 0; i < image.data.length / 3; i++) {
            png.data[i * 4] = image.data[i * 3];
            png.data[i * 4 + 1] = image.data[i * 3 + 1];
            png.data[i * 4 + 2] = image.data[i * 3 + 2];
            png.data[i * 4 + 3] = 255;
        }
    
        png.pack().pipe(fs.createWriteStream("test.png"));
    }
});

// console.log(image);

// xsc.startAutoCapture(100, -1, image => console.log(image));

// setTimeout(() => xsc.stopAutoCapture(), 2000);