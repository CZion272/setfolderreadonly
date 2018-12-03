#include <stdio.h>
#include <node.h>
#include <Windows.h>
#include <list>

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
		FILE *pFformat = NULL;
		FILE *pFmetadate = NULL;
		FILE *pFImage = NULL;
		const char* chPath;

		bool open(const char *chFilePath)
		{
			pFformat = NULL;
			pFmetadate = NULL;
			pFImage = NULL;
			string strFilePath = chFilePath;
			strFilePath += "\\formats.json";
			fopen_s(&pFformat, strFilePath.c_str(), "r");
			if (!pFformat)
			{
				return false;
			}

			strFilePath = chFilePath;
			strFilePath += "\\metadata.json";
			fopen_s(&pFmetadate, strFilePath.c_str(), "r");
			if (!pFmetadate)
			{
				fclose(pFformat);
				pFformat = NULL;
				return false;
			}

			strFilePath = chFilePath;
			strFilePath += "\\image\\.image";
			fopen_s(&pFImage, strFilePath.c_str(), "w");
			if (!pFImage)
			{
				fclose(pFformat);
				pFformat = NULL;
				fclose(pFmetadate);
				pFmetadate = NULL;
				return false;
			}
			chPath = chFilePath;
		}

		void close()
		{
			if (pFformat)
			{
				fclose(pFformat);
			}
			if (pFmetadate)
			{
				fclose(pFmetadate);
			}
			fclose(pFmetadate);
			if (pFImage)
			{
				fclose(pFImage);
			}
		}
	};

	list<FileHandle> g_listFiles;

	string findInstallPath(string strProgram)
	{
		string strLocal = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
		strLocal += strProgram;
		HKEY hkResult;
		
		char szBuffer[MAX_PATH];
		ZeroMemory(&szBuffer, sizeof(char));
		DWORD dwNameLen = MAX_PATH;
		DWORD dwType = REG_BINARY | REG_DWORD | REG_EXPAND_SZ | REG_MULTI_SZ | REG_NONE | REG_SZ;
		
		if (ERROR_SUCCESS ==
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, strLocal.c_str(), 0, KEY_READ,	&hkResult))
		{
			if (ERROR_SUCCESS ==
				RegQueryValueEx(hkResult, "", 0, &dwType, (LPBYTE)szBuffer, &dwNameLen))
			{				
				string strBuffer = szBuffer;

				return strBuffer;
			};			
		}
		return "";
	}
	
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
		list<FileHandle>::iterator iter;
		for (iter = g_listFiles.begin(); iter != g_listFiles.end(); iter++)
		{
			if (strcmp(iter->chPath, chFilePath) == 0)
			{
				args.GetReturnValue().Set(FALSE);
				return;
			}
		}

		FileHandle files;
		ZeroMemory(&files, sizeof(FileHandle));

		if (!files.open(chFilePath))
		{
			args.GetReturnValue().Set(FALSE);
			return;
		}
		g_listFiles.push_back(files);
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

		list<FileHandle>::iterator iter;
		for (iter = g_listFiles.begin(); iter != g_listFiles.end(); iter++)
		{
			if (strcmp(iter->chPath, chpath) == 0)
			{
				iter->close();
				g_listFiles.erase(iter);
				args.GetReturnValue().Set(TRUE);
				return;
			}
		}
		args.GetReturnValue().Set(FALSE);
		return;
	}

	void findInstall(const FunctionCallbackInfo<Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (!args[0]->IsString())
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			return;
		}
		v8::String::Utf8Value str(args[0]->ToString());
		const char *chProgrameName = *str;
		string strPrograme = chProgrameName;
		string strIntallPath = findInstallPath(strPrograme);

		args.GetReturnValue().Set(strIntallPath == "" ? FALSE : TRUE);
		return;
	}
	
	void openWithPrograme(const FunctionCallbackInfo<Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (!args[0]->IsString() || !args[1]->IsString())
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			return;
		}
		v8::String::Utf8Value str(args[0]->ToString());
		const char *chProgrameName = *str;
		string strPrograme = chProgrameName;
		
		v8::String::Utf8Value str1(args[1]->ToString());
		const char *chOpenFile = *str1;
		string strOpenFile = chOpenFile;
		
		string strLocal = findInstallPath(strPrograme);
		if(strLocal != "")
		{
			strLocal += " ";
			strLocal += strOpenFile;
			
			WinExec(strLocal.c_str(), SW_SHOW);
			
			args.GetReturnValue().Set(TRUE);
			return;
		}

		args.GetReturnValue().Set(FALSE);
		return;
	}

	void Init(Local<Object> exports)
	{
		NODE_SET_METHOD(exports, "openFile", openFile);
		NODE_SET_METHOD(exports, "closeFile", closeFile);
		NODE_SET_METHOD(exports, "findInstall", findInstall);
		NODE_SET_METHOD(exports, "openWithPrograme", openWithPrograme);
	}
	NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
}