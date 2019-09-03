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

More examples can be found in the _examples/_ directory.

# Methods

## Data format

All of the method returning images return an object with the format:

```javascript
{
	data: Buffer,
	width: Number,
	height: Number
}
```

The data in the buffer are the raw pixel values either in RGB or RGBA order.
This object format is referred to as the "default format" in the rest of this documentation.

## XScreencap

**constructor**(?pixel_format)  
Create a new instance with an optional pixel format.
The pixel format determines the data contained in the returned buffers and can either be `"RGB"` or `"RGBA"` (default).
The alpha channel is always set to 255, however.

**connect**()  
This method connects the library to the X server and it needs to be called before any other method.
Returns `true` on success and `false` if something went wrong.

**getMonitorCount**()  
Returns the number of active monitors identified via RandR.
If RandR is not available, this method returns `-1`.

**getImage**(monitor)  
Synchronously gets the image of the specified monitor.
If RandR is not available, or `-1` is supplied as the monitor value, the whole virtual X screen will be captured.
The method either returns an image in the default format or throws an error.

**getImageAsync**(monitor)  
Asynchronous version of `getImage`, which returns a promise resolving to image data in the default format.

**startAutoCapture**(delay, monitor, ?allowSkips)  
Starts a new thread, which tries to capture the screen every `delay` milliseconds.
Image data is then emitted as an **image** event.
This method functions similar to `setInterval(() => xsc.getImageAsync().then(image => emit("image", image)), delay)`, but with the added bonus of all the timing stuff happening in native code and a separate thread, which improves the performance. **Note**: You can only have one of these threads running at any time, so subsequent calls to `startAutoCapture` without stopping the auto capture in between have no effect.
The optional parameter `allowSkips` controls how the thread queues up the **image** events.
If the event did not have a chance to fire before the next image is captured, it can either be queued up (`allowSkips = false`) or just be thrown away (`allowSkips = true`, default).

**stopAutoCapture**(?clearBacklog)  
Stops the auto capture thread.
By default, no futher **image** events will be emitted after this method has been called, since `clearBacklog` is `true` by default.
If you want to process every captured frame however, set `clearBacklog` to `false`.

# Events

Event **'image'**  
Emitted in an interval which duration is determined by the delay parameter in the `startAutoCapture` method.
The event handler will be called with an object in the default image format.