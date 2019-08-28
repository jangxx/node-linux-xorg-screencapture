#include "xscreencap.h"

XScreencap::XScreencap(const Napi::CallbackInfo &info) : Napi::ObjectWrap<XScreencap>(info), m_Display(NULL), m_autoCaptureThreadStarted(false)
{

}

XScreencap::~XScreencap() {
    if (m_Display != NULL) {
        XCloseDisplay(m_Display);
    }

    if (m_autoCaptureThreadStarted) {
        m_autoCaptureThreadSignal.set_value();

        m_autoCaptureThread.join(); // wait for thread to finish
    }
}

Napi::Value XScreencap::connect(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    m_Display = XOpenDisplay(NULL);

    if (m_Display == NULL) {
        return Napi::Boolean::New(env, false);
    }

    m_RootWindow = XDefaultRootWindow(m_Display);

    Status status = XGetWindowAttributes(m_Display, m_RootWindow, &m_WinAttr);

    if (status != 0) {
        return Napi::Boolean::New(env, true);
    } else {
        return Napi::Boolean::New(env, false);
    }
}

XImage* XScreencap::getImage(int monitor = -1) {
    if (m_Display == NULL) return NULL;

    int x = 0, y = 0, width = m_WinAttr.width, height = m_WinAttr.height;

    #if XRANDR_MONITORS_SUPPORTED()

    int nmonitors;
    XRRMonitorInfo* info = XRRGetMonitors(m_Display, m_RootWindow, false, &nmonitors);

    if (monitor >= 0 && monitor < nmonitors) {
        x = info[monitor].x;
        y = info[monitor].y;
        width = info[monitor].width;
        height = info[monitor].height;
    }

    XRRFreeMonitors(info);

    #endif

    XImage* image = XGetImage(m_Display, m_RootWindow, x, y, width, height, 0xFFFFFFFF, ZPixmap);

    return image;
}

Napi::Value XScreencap::wrap_getImage(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    int monitor = info[0].As<Napi::Number>().Int32Value();

    Napi::Object result = Napi::Object::New(env);
    
    XImage* image = getImage(monitor);

    if (image == NULL) {
        result.Set("error", "Not connected to X server");
        return result;
    }

    void* data = malloc(image->width * image->height * 3);
    char* data_pointer = reinterpret_cast<char*>(data);

    for (int x = 0; x < image->width; x++) {
        for (int y = 0; y < image->height; y++) {
            long pixel = XGetPixel(image, x, y);
            
            char red = (image->byte_order == MSBFirst) ? (pixel & image->red_mask) : (pixel & image->red_mask) >> 16;
            char green = (pixel & image->green_mask) >> 8;
            char blue = (image->byte_order == MSBFirst) ? (pixel & image->blue_mask) >> 16 : (pixel & image->blue_mask);

            data_pointer[(x + y * image->width)*3] = red;
            data_pointer[(x + y * image->width)*3 + 1] = green;
            data_pointer[(x + y * image->width)*3 + 2] = blue;
        }
    }

    Napi::Buffer<char> buf = Napi::Buffer<char>::New(env, data_pointer, image->width * image->height * 3, [](Napi::Env env, char* data) { free(data); });

    result.Set("error", env.Null());
    result.Set("width", Napi::Number::New(env, (double)image->width));
    result.Set("height", Napi::Number::New(env, (double)image->height));
    result.Set("data", buf);

    XDestroyImage(image);

    return result;
}

Napi::Value XScreencap::getMonitorCount(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    int nmonitors = -1;

    #if XRANDR_MONITORS_SUPPORTED()

    XRRMonitorInfo* monitorInfo = XRRGetMonitors(m_Display, m_RootWindow, false, &nmonitors);
    XRRFreeMonitors(monitorInfo);

    #endif

    return Napi::Number::New(env, (double)nmonitors);
}

Napi::Value XScreencap::startAutoCapture(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (m_autoCaptureThreadStarted) {
        return Napi::Boolean::New(env, false);
    }

    int delay = info[0].As<Napi::Number>().Int32Value();
    int monitor = info[1].As<Napi::Number>().Int32Value();
    Napi::Function callback = info[2].As<Napi::Function>();

    m_autoCaptureThreadCallback = Napi::ThreadSafeFunction::New(env, callback, "AutoCaptureThreadCallback", 0, 1);

    m_autoCaptureThreadSignal = std::promise<void>();

    m_autoCaptureThread = std::thread(&XScreencap::autoCaptureFn, this, delay, monitor);

    m_autoCaptureThreadStarted = true;

    return Napi::Boolean::New(env, true);
}

Napi::Value XScreencap::stopAutoCapture(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (!m_autoCaptureThreadStarted) {
        return Napi::Boolean::New(env, false);
    }

    m_autoCaptureThreadSignal.set_value();

    m_autoCaptureThread.join(); // wait for thread to finish

    m_autoCaptureThreadCallback.Release();

    m_autoCaptureThreadStarted = false;

    return Napi::Boolean::New(env, true);
}

void XScreencap::autoCaptureFn(int delay, int monitor) {
    auto callback = [](Napi::Env env, Napi::Function fn, RESULT_TRANSPORT* resultRaw) {
        Napi::Object result = Napi::Object::New(env);
        result.Set("width", Napi::Number::New(env, (double)resultRaw->width));
        result.Set("height", Napi::Number::New(env, (double)resultRaw->height));

        Napi::Buffer<char> buf = Napi::Buffer<char>::New(env, resultRaw->data, resultRaw->width * resultRaw->height * 3, [](Napi::Env env, char* data) { free(data); });

        result.Set("data", buf);

        fn.Call({ result });
    };

    std::future<void> signal = m_autoCaptureThreadSignal.get_future();

    while (signal.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        auto start = std::chrono::high_resolution_clock::now();

        XImage* image = getImage(monitor);

        if (image == NULL) {
            // ignore error case
            auto finish = std::chrono::high_resolution_clock::now();

            auto exTime = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

            if (exTime.count() < delay - 1) {
                auto waitTime = std::chrono::milliseconds{ delay - 1 } - exTime;
                std::this_thread::sleep_for(waitTime); // wait the rest of the delay until the next screen capture
            }

            continue;
        }

        void* data = malloc(image->width * image->height * 3);
        char* data_pointer = reinterpret_cast<char*>(data);

        for (int x = 0; x < image->width; x++) {
            for (int y = 0; y < image->height; y++) {
                long pixel = XGetPixel(image, x, y);
                
                char red = (image->byte_order == MSBFirst) ? (pixel & image->red_mask) : (pixel & image->red_mask) >> 16;
                char green = (pixel & image->green_mask) >> 8;
                char blue = (image->byte_order == MSBFirst) ? (pixel & image->blue_mask) >> 16 : (pixel & image->blue_mask);

                data_pointer[(x + y * image->width)*3] = red;
                data_pointer[(x + y * image->width)*3 + 1] = green;
                data_pointer[(x + y * image->width)*3 + 2] = blue;
            }
        }

        RESULT_TRANSPORT result;

        result.data = data_pointer;
        result.width = image->width;
        result.height = image->height;

        XDestroyImage(image);

        m_autoCaptureThreadCallback.NonBlockingCall( &result, callback );

        auto finish = std::chrono::high_resolution_clock::now();

        // check if have to finish before waiting for a potentially long time
        if (signal.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
            break;
        }

        auto exTime = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

        if (exTime.count() < delay - 1) {
            auto waitTime = std::chrono::milliseconds{ delay - 2 } - exTime; // subtract 2ms to account for the signal waiting
            std::this_thread::sleep_for(waitTime); // wait the rest of the delay until the next screen capture
        }
    }
}

Napi::FunctionReference XScreencap::constructor;

Napi::Object XScreencap::Init(Napi::Env env, Napi::Object exports) {
	Napi::Function func = DefineClass(env, "XScreencap", {
		InstanceMethod("connect", &XScreencap::connect),
		InstanceMethod("getImage", &XScreencap::wrap_getImage),
        InstanceMethod("getMonitorCount", &XScreencap::getMonitorCount),
        InstanceMethod("startAutoCapture", &XScreencap::startAutoCapture),
        InstanceMethod("stopAutoCapture", &XScreencap::stopAutoCapture)
    });

	constructor = Napi::Persistent(func);

	constructor.SuppressDestruct();

	exports.Set("XScreencap", func);
	return exports;
}

Napi::Object Init (Napi::Env env, Napi::Object exports) {
    XScreencap::Init(env, exports);
    return exports;
}


NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)