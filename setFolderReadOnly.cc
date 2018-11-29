#include <stdio.h>
#include <node.h>
#include <Windows.h>
#include <map>
	
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;
using namespace std;
namespace addons
{
	struct FileHandle
	{
		FILE *pFformat;
		FILE *pFmetadate;
		FILE *pFImage;
		
		bool open(const char *chFilePath)
		{
			pFformat = NULL;
			pFmetadate = NULL;
			pFImage = NULL;
			string strFilePath = chFilePath;
			strFilePath += "\\formats.json";
			fopen_s(&pFformat, strFilePath.c_str(), "r");
			if(!pFformat)
			{
				return false;
			}

			strFilePath = chFilePath;
			strFilePath += "\\metadata.json";
			fopen_s(&pFmetadate, strFilePath.c_str(), "r");
			if(!pFmetadate)
			{
				fclose(pFformat);
				return false;
			}

			strFilePath = chFilePath;
			strFilePath += "\\image\\.image";
			fopen_s(&pFImage, strFilePath.c_str(), "w");
			if(!pFImage)
			{
				fclose(pFformat);
				fclose(pFmetadate);
				return false;
			}
		}
		
		void close()
		{
			fclose(pFformat);
			fclose(pFmetadate);
			fclose(pFImage);
		}
	};

	map<const char*, FileHandle*> g_mapFiles;

	void openFile(const FunctionCallbackInfo<Value>& args)
	{
		Isolate* isolate = args.GetIsolate();

		if (!args[0]->IsString())
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			args.GetReturnValue().Set(FALSE);
			return;
		}
		v8::String::Utf8Value str(args[0]->ToString());

		const char *chFilePath = *str;
		map<const char*, FileHandle*>::iterator iter;
		for (iter = g_mapFiles.begin(); iter != g_mapFiles.end(); iter++)
		{
			if (strcmp(chFilePath, iter->first))
			{
				args.GetReturnValue().Set(FALSE);
				return;
			}
		}
		FILE *f = NULL;
		FILE *f1 = NULL;
		FILE *f2 = NULL;
		
		FileHandle *files = new FileHandle;
		ZeroMemory(files, sizeof(FileHandle));
		
		if (!files->open(chFilePath))
		{
			args.GetReturnValue().Set(FALSE);
			return;
		}
		g_mapFiles.insert(pair<const char*, FileHandle*>(chFilePath, files));
		args.GetReturnValue().Set(TRUE);
	}

	void closeFile(const FunctionCallbackInfo<Value>& args)
	{
		Isolate* isolate = args.GetIsolate();

		if (!args[0]->IsString())
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			return;
		}
		v8::String::Utf8Value str(args[0]->ToString());
		const char *chpath = *str;
		map<const char*, FileHandle*>::iterator iter;
		for (iter = g_mapFiles.begin(); iter != g_mapFiles.end(); iter++)
		{
			if (strcmp(chpath, iter->first))
			{
				iter->second->close();
				args.GetReturnValue().Set(TRUE);
				return;
			}
		}
		args.GetReturnValue().Set(FALSE);
		return;
	}

	void Init(Local<Object> exports)
	{
		NODE_SET_METHOD(exports, "openFile", openFile);
		NODE_SET_METHOD(exports, "closeFile", closeFile);
	}
	NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
}