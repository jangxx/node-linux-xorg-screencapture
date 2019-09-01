#pragma once

#include "napi.h"
#include "types.h"
#include "xscreencap.h"

class XScreencap;

class GetFrameAsyncWorker : public Napi::AsyncWorker {
	public:
		GetFrameAsyncWorker(XScreencap* target, int monitor, PIXEL_FORMAT format, int formatSize, Napi::Function callback);

		void Execute();

		std::vector<napi_value> GetResult(Napi::Env env);
		
	private:
		RESULT_TRANSPORT m_Result;
		int m_Monitor;
		XScreencap* m_XSc;
		PIXEL_FORMAT m_Format;
		int m_FormatSize;
};