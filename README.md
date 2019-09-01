# Linux Xorg Screencapture
A native Node.js addon which captures the screen content on Linux using the Xorg API

# Installation

    npm install linux-xorg-screencapture

# Usage

```javascript
const { XScreencap } = require('linux-xorg-screencapture');

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

// work with the data in image
```

# Methods

**coming soon**