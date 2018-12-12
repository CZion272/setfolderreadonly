#include <stdio.h>
#include <node.h>
#include <Windows.h>
#include <list>
#include <QString>
#include <QList>
#include <QDebug>
#include <Shlobj_core.h>
#include <QTemporaryFile>
#include <olectl.h>
#include <QProcess>
#pragma comment(lib, "oleaut32.lib")

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

	HRESULT SaveIcon(HICON hIcon, const wchar_t* path) {
		// Create the IPicture intrface
		PICTDESC desc = { sizeof(PICTDESC) };
		desc.picType = PICTYPE_ICON;
		desc.icon.hicon = hIcon;
		IPicture* pPicture = 0;
		HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
		if (FAILED(hr)) return hr;

		// Create a stream and save the image
		IStream* pStream = 0;
		CreateStreamOnHGlobal(0, TRUE, &pStream);
		LONG cbSize = 0;
		hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);

		// Write the stream content to the file
		if (!FAILED(hr)) {
			HGLOBAL hBuf = 0;
			GetHGlobalFromStream(pStream, &hBuf);
			void* buffer = GlobalLock(hBuf);
			HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
			if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
			else {
				DWORD written = 0;
				WriteFile(hFile, buffer, cbSize, &written, 0);
				CloseHandle(hFile);
			}
			GlobalUnlock(buffer);
		}
		// Cleanup
		pStream->Release();
		pPicture->Release();
		return hr;

	}

	struct FileHandle
	{
		FILE *pFImage = NULL;
		QString strPath;

		bool open(QString strFilePath)
		{
			pFImage = NULL;
			QString strOpenFilePath = strFilePath.replace("/", "\\");
			strOpenFilePath += "\\image\\.image";
			fopen_s(&pFImage, strOpenFilePath.toLatin1().constData(), "w");
			if (!pFImage)
			{
				return false;
			}
			strPath = strFilePath;
			return true;
		}

		void close()
		{
			if (pFImage)
			{
				fclose(pFImage);
				pFImage = NULL;
			}
		}
	};

	QList<FileHandle> g_listFiles;

	QString findInstallPath(QString strProgram, QString &strIconFile)
	{
		QString strLocal = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
		strLocal += strProgram;
		HKEY hkResult;

		char szBuffer[MAX_PATH];
		ZeroMemory(&szBuffer, sizeof(char));
		DWORD dwNameLen = MAX_PATH;
		DWORD dwType = REG_BINARY | REG_DWORD | REG_EXPAND_SZ | REG_MULTI_SZ | REG_NONE | REG_SZ;

		if (ERROR_SUCCESS ==
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, strLocal.toLatin1().constData(), 0, KEY_READ, &hkResult))
		{
			if (ERROR_SUCCESS ==
				RegQueryValueEx(hkResult, "", 0, &dwType, (LPBYTE)szBuffer, &dwNameLen))
			{
				return QString(szBuffer);
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
		for (int i = 0; i < g_listFiles.count(); i++)
		{
			if (g_listFiles.at(i).strPath == strFilePath)
			{
				args.GetReturnValue().Set(FALSE);
				return;
			}
		}
		FileHandle files;

		if (!files.open(strFilePath.toLatin1().constData()))
		{
			args.GetReturnValue().Set(FALSE);
			return;
		}
		g_listFiles.append(files);
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

		for (int i = 0; i < g_listFiles.count(); i++)
		{
			if (g_listFiles.at(i).strPath == strFilePath)
			{
				FileHandle file = g_listFiles.at(i);
				file.close();
				g_listFiles.removeAt(i);
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
		QString strIcon;
		QString strIntallPath = findInstallPath(strProgrameName, strIcon);
		qDebug() << strIcon;
		args.GetReturnValue().Set(strIntallPath == "" ? FALSE : TRUE);
		return;
	}

	void openWithPrograme(const FunctionCallbackInfo<Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (!args[0]->IsString() || !args[1]->IsNumber())
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误:使用int类型作为第二个参数，0表示默认程序，其他表示以其他方式打开")));
			return;
		}

		v8::String::Utf8Value str(args[0]->ToString());
		QString strOpenFile = *str;
		int nMode = args[1]->IntegerValue();
		wchar_t wcFile[MAX_PATH] = { 0 };
		strOpenFile.toWCharArray(wcFile);

		if (nMode == 0)
		{
			ShellExecuteW(NULL, L"open", wcFile, NULL, NULL, SW_SHOW);
		}
		else
		{
			QString argm = "rundll32 shell32,OpenAs_RunDLL ";
			argm = argm + strOpenFile;

			std::string strArgm = argm.toStdString();
			const char* ch = strArgm.c_str();
			QProcess::execute(argm);
		}
		args.GetReturnValue().Set(TRUE);
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