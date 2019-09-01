const { XScreencap } = require('..');
const { PNG } = require('pngjs');
const fs = require('fs');

let xsc = new XScreencap("RGBA");

let success = xsc.connect();

if (!success) {
	console.log("Could not connect to X server");
	process.exit(0);
}

let image;

try {
	image = xsc.getImage(-1);
} catch(err) {
	console.log("An error occured:", err.message);
	process.exit(0);
}

let png = new PNG({
	width: image.width,
	height: image.height
});

image.data.copy(png.data);

png.pack().pipe(fs.createWriteStream("test.png"));