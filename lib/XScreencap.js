const { XScreencap: XScreencap_native } = require('../build/Release/xscreencap');
const { EventEmitter } = require('events');

class XScreencap extends EventEmitter {
    constructor(pixel_format = "RGBA") {
        super();

        this._xsc = new XScreencap_native(pixel_format);

        this._autoCaptureStarted = false;
        this._clearBacklog = true;
    }

    connect() {
        return this._xsc.connect();
    }

    getMonitorCount() {
        return this._xsc.getMonitorCount();
    }

    getImage(monitor) {
        let image = this._xsc.getImage(monitor);

        if (image.error != null) {
            throw new Error(image.error);
        }

        return {
            data: image.data,
            width: image.width,
            height: image.height
        };
    }

    getImageAsync(monitor) {
        return new Promise((resolve, reject) => {
            this._xsc.getImageAsync(monitor, image => {
                if (image.error != null) {
                    return reject(new Error(image.error));
                } else {
                    return resolve({
                        data: image.data,
                        width: image.width,
                        height: image.height
                    });
                }
            });
        });
    }

    startAutoCapture(delay, monitor, allowSkips=true) {
        if (this._autoCaptureStarted) return;

        this._xsc.startAutoCapture(delay, monitor, allowSkips, image => {
            if (!this._autoCaptureStarted && this._clearBacklog) return; // prevent some stray functions in the global event queue from emitting events afte rthe capture thread has been stopped

            setImmediate(() => {
                if (image.error == null) {
                    this.emit("image", {
                        data: image.data,
                        width: image.width,
                        height: image.height
                    });
                }
            });
        });

        this._autoCaptureStarted = true;
    }

    stopAutoCapture(clearBacklog=true) {
        if (!this._autoCaptureStarted) return;

        this._clearBacklog = clearBacklog;

        this._xsc.stopAutoCapture();
        this._autoCaptureStarted = false;
    }
}

module.exports = { XScreencap };