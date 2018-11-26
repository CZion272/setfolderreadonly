#include <stdio.h>
#include <node.h>
#include <Windows.h>
	
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

void setFolderReadOnly(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	if (!args[0]->IsString())
	{
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "参数错误")));
		return;
	}
	v8::String::Utf8Value str(args[0]->ToString());
	char *chFilePath = *str;
	FILE *f = NULL;
	fopen_s(&f, chFilePath, "r");
	if (f)
	{
		fclose(f);
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "参数错误")));
		return;
	}
	BOOL bSet = SetFileAttributes(chFilePath, FILE_ATTRIBUTE_READONLY);
	//Local<Number> b = Number::new(isolate, bSet);
	args.GetReturnValue().Set(bSet);
}

void Init(Local<Object> exports)
{
	NODE_SET_METHOD(exports, "setFolderReadOnly", setFolderReadOnly);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)