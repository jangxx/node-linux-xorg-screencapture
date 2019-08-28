#pragma once

#include "napi.h"

#include <chrono>
#include <thread>
#include <future>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#define XRANDR_MONITORS_SUPPORTED() ((RANDR_MAJOR > 1) || (RANDR_MAJOR == 1 && RANDR_MINOR > 5))

typedef struct {
	char* data;
	int width;
	int height;
} RESULT_TRANSPORT;

class XScreencap : public Napi::ObjectWrap<XScreencap> {
	public:
		static Napi::Object Init(Napi::Env env, Napi::Object exports);

		XScreencap(const Napi::CallbackInfo &info);
		~XScreencap();

		Napi::Value connect(const Napi::CallbackInfo &info);

		XImage* getImage(int monitor);

		Napi::Value wrap_getImage(const Napi::CallbackInfo &info);

		Napi::Value getMonitorCount(const Napi::CallbackInfo &info);

		Napi::Value startAutoCapture(const Napi::CallbackInfo &info);

		Napi::Value stopAutoCapture(const Napi::CallbackInfo &info);

	private:
		static Napi::FunctionReference constructor;

		void autoCaptureFn(int delay, int monitor);

		Display* m_Display;
		Window m_RootWindow;
		XWindowAttributes m_WinAttr;
		std::thread m_autoCaptureThread;
		bool m_autoCaptureThreadStarted;
		std::promise<void> m_autoCaptureThreadSignal;
		Napi::ThreadSafeFunction m_autoCaptureThreadCallback;
};