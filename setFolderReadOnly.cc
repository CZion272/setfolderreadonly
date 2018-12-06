#include <stdio.h>
#include <node.h>
#include <Windows.h>
#include <list>
#include <QString>
#include <QDebug>

using v8::Context;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
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
		QString strPath;

		bool open(QString strFilePath)
		{
			pFformat = NULL;
			pFmetadate = NULL;
			pFImage = NULL;
			QString strOpenFilePath = strFilePath;
			strOpenFilePath += "\\formats.json";
			fopen_s(&pFformat, strFilePath.toLatin1().constData(), "r");
			if (!pFformat)
			{
				return false;
			}

			strOpenFilePath = strFilePath;
			strOpenFilePath += "\\metadata.json";
			fopen_s(&pFmetadate, strFilePath.toLatin1().constData(), "r");
			if (!pFmetadate)
			{
				fclose(pFformat);
				pFformat = NULL;
				return false;
			}

			strOpenFilePath = strFilePath;
			strOpenFilePath += "\\image\\.image";
			fopen_s(&pFImage, strFilePath.toLatin1().constData(), "w");
			if (!pFImage)
			{
				fclose(pFformat);
				pFformat = NULL;
				fclose(pFmetadate);
				pFmetadate = NULL;
				return false;
			}
			strPath = strFilePath;
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
	   
	QString findInstallPath(QString strProgram)
	{
		QString strLocal = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
		strLocal += strProgram;
		HKEY hkResult;
		
		char szBuffer[MAX_PATH];
		ZeroMemory(&szBuffer, sizeof(char));
		DWORD dwNameLen = MAX_PATH;
		DWORD dwType = REG_BINARY | REG_DWORD | REG_EXPAND_SZ | REG_MULTI_SZ | REG_NONE | REG_SZ;
		
		if (ERROR_SUCCESS ==
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, strLocal.toLatin1().constData(), 0, KEY_READ,	&hkResult))
		{
			if (ERROR_SUCCESS ==
				RegQueryValueEx(hkResult, "", 0, &dwType, (LPBYTE)szBuffer, &dwNameLen))
			{				
				QString strBuffer = szBuffer;

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

		QString strFilePath = *str;
		list<FileHandle>::iterator iter;
		for (iter = g_listFiles.begin(); iter != g_listFiles.end(); iter++)
		{
			if (iter->strPath == strFilePath)
			{
				args.GetReturnValue().Set(FALSE);
				return;
			}
		}

		FileHandle files;
		ZeroMemory(&files, sizeof(FileHandle));

		if (!files.open(strFilePath.toLatin1().constData()))
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
		
		QString strFilePath = *str;

		list<FileHandle>::iterator iter;
		for (iter = g_listFiles.begin(); iter != g_listFiles.end(); iter++)
		{
			if (iter->strPath == strFilePath)
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
		QString strProgrameName = *str;

		QString strIntallPath = findInstallPath(strProgrameName);

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
		QString strPrograme = *str;
		
		v8::String::Utf8Value str1(args[1]->ToString());
		QString strOpenFile = *str1;
		qDebug() << strPrograme;
		QString strLocal = findInstallPath(strPrograme);
		if(strLocal != "")
		{
			strLocal += " ";
			strLocal += strOpenFile.toLatin1().constData();
			
			WinExec(strLocal.toLatin1().constData(), SW_SHOW);
			
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