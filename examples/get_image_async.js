const { XScreencap } = require('..');
const { PNG } = require('pngjs');
const fs = require('fs');

let xsc = new XScreencap("RGBA");

let success = xsc.connect();

if (!success) {
	console.log("Could not connect to X server");
	process.exit(0);
}

xsc.getImageAsync(-1).then(image => {
	let png = new PNG({
		width: image.width,
		height: image.height
	});
	
	image.data.copy(png.data);
	
	png.pack().pipe(fs.createWriteStream("test.png"));
}).catch(err => console.log("An error occured:", err.message));