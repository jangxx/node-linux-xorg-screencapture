#include "getframeasyncworker.h"

GetFrameAsyncWorker::GetFrameAsyncWorker(XScreencap* target, int monitor, PIXEL_FORMAT format, int formatSize, Napi::Function callback) : 
	Napi::AsyncWorker(callback), 
	m_Monitor(monitor), 
	m_XSc(target), 
	m_Format(format),
	m_FormatSize(formatSize)
{ }

void GetFrameAsyncWorker::Execute() {
    XImage* image = m_XSc->getImage(m_Monitor);

    if (image == NULL) {
		m_Result.error = "Not connected to X server";
        return;
    }

    char* data = XScreencap::getImageData(image, m_Format);

	m_Result.data = data;
	m_Result.width = image->width;
	m_Result.height = image->height;

    XDestroyImage(image);
}

std::vector<napi_value> GetFrameAsyncWorker::GetResult(Napi::Env env) {
	Napi::Object result = Napi::Object::New(env);

	if (m_Result.error == "") {
		result.Set("error", env.Null());
	} else {
		result.Set("error", m_Result.error);
	}
	
	Napi::Buffer<char> buf = Napi::Buffer<char>::New(env, m_Result.data, m_Result.width * m_Result.height * m_FormatSize, [](Napi::Env env, char* data) { free(data); });

	result.Set("error", env.Null());
    result.Set("width", Napi::Number::New(env, (double)m_Result.width));
    result.Set("height", Napi::Number::New(env, (double)m_Result.height));
    result.Set("data", buf);

	return { result };	
}