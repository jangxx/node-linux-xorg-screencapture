const { XScreencap } = require('..');

let xsc = new XScreencap("RGBA");

let success = xsc.connect();

if (!success) {
	console.log("Could not connect to X server");
	process.exit(0);
}

let monitors = xsc.getMonitorCount();

if (monitors == -1) {
	console.log("Could not get number of monitors");
} else {
	console.log(`Found ${monitors} monitor(s)`);
}