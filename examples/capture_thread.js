const { XScreencap } = require('../');
const { PNG } = require('pngjs');
const fs = require('fs');

let xsc = new XScreencap("RGBA");

let success = xsc.connect();

if (!success) {
	console.log("Could not connect to X server");
	process.exit(0);
}

xsc.startAutoCapture(100, 0, false);

let imageCount = 0;
let pngs = [];

xsc.on("image", image => {
	let png = new PNG({
		width: image.width,
		height: image.height
	});
	
	image.data.copy(png.data);

	pngs.push(png);

	imageCount++;
});

setTimeout(() => {
	xsc.stopAutoCapture(true);

	console.log("Captured", imageCount, "images");

	for(let p in pngs) {
		pngs[p].pack().pipe(fs.createWriteStream(`test_image_${p}.png`));
	}
}, 2000);