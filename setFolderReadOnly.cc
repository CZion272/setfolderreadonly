#include <stdio.h>
#define NAPI_VERSION 2
#include <napi.h>
#include <Windows.h>
#include <list>

using namespace std;

namespace addons
{
	struct FileHandle
	{
		FILE *pFformat = NULL;
		FILE *pFmetadate = NULL;
		FILE *pFImage = NULL;
		string chPath;

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
			return true;
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
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, strLocal.c_str(), 0, KEY_READ, &hkResult))
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

	Napi::Boolean openFile(const Napi::CallbackInfo& info)
	{
		Napi::Env env = info.Env();

		if (!info[0].IsString())
		{
			Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, FALSE);
		}

		string strFilePath = info[0].ToString();
		list<FileHandle>::iterator iter;
		for (iter = g_listFiles.begin(); iter != g_listFiles.end(); iter++)
		{
			if (iter->chPath == strFilePath)
			{
				return Napi::Boolean::New(env, FALSE);
			}
		}

		FileHandle files;
		ZeroMemory(&files, sizeof(FileHandle));

		if (!files.open(strFilePath.c_str()))
		{
			return Napi::Boolean::New(env, FALSE);
		}
		g_listFiles.push_back(files);
		return Napi::Boolean::New(env, TRUE);
	}

	Napi::Boolean closeFile(const Napi::CallbackInfo& info)
	{
		Napi::Env env = info.Env();

		if (!info[0].IsString())
		{
			Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, FALSE);
		}
		string strPath = info[0].ToString();

		list<FileHandle>::iterator iter;
		for (iter = g_listFiles.begin(); iter != g_listFiles.end(); iter++)
		{
			if (iter->chPath == strPath)
			{
				iter->close();
				g_listFiles.erase(iter);
				return Napi::Boolean::New(env, TRUE);
			}
		}
		return Napi::Boolean::New(env, FALSE);
	}

	Napi::Boolean findInstall(const Napi::CallbackInfo& info)
	{
		Napi::Env env = info.Env();
		if (!info[0].IsString())
		{
			Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, FALSE);
		}
		string strPrograme = info[0].ToString();
		string strIntallPath = findInstallPath(strPrograme);

		return Napi::Boolean::New(env,(strIntallPath == "" ? FALSE : TRUE));
	}

	Napi::Boolean openWithPrograme(const Napi::CallbackInfo& info)
	{
		Napi::Env env = info.Env();
		if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString())
		{
			Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, FALSE);
		}

		string strPrograme = info[0].ToString();

		string strOpenFile = info[1].ToString();

		string strLocal = findInstallPath(strPrograme);
		if (strLocal != "")
		{
			strLocal += " ";
			strLocal += strOpenFile;

			WinExec(strLocal.c_str(), SW_SHOW);

			return Napi::Boolean::New(env, TRUE);
		}

		return Napi::Boolean::New(env, FALSE);
	}

	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
		exports.Set(Napi::String::New(env, "openFile"),
              Napi::Function::New(env, openFile));
		exports.Set(Napi::String::New(env, "closeFile"),
              Napi::Function::New(env, closeFile));
		exports.Set(Napi::String::New(env, "findInstall"),
              Napi::Function::New(env, findInstall));
		exports.Set(Napi::String::New(env, "openWithPrograme"),
              Napi::Function::New(env, openWithPrograme));
		return exports;
	}
	
	NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
}