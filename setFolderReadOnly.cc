#include <stdio.h>
#include <node_api.h>

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

#include <list>
#include <QString>
#include <QList>
#include <QDebug>
#include <Shlobj_core.h>
#include <QTemporaryFile>
#include <olectl.h>
#include <QProcess>
#include <vector>
#include <bitset>
#include <array>
#include <intrin.h>
#include <QClipboard>
#include <QMimeData>
#include <QGuiApplication>
#include <QThread>
#pragma comment(lib, "oleaut32.lib")

using namespace std;

#define DECLARE_NAPI_METHOD(name, func)                          \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

namespace addons
{
    static napi_value intValue(napi_env env, int num)
    {
        napi_value rb;
        napi_create_int32(env, num, &rb);
        return rb;
    }

    static napi_value stringValue(napi_env env, QString str)
    {
        napi_value rb;
        napi_create_string_latin1(env, str.toLatin1(), str.length(), &rb);
        return rb;
    }

    static napi_value boolenValue(napi_env env, bool b)
    {
        napi_value rb;
        napi_get_boolean(env, b, &rb);
        return rb;
    }

    HRESULT SaveIcon(HICON hIcon, const wchar_t* path)
    {
#ifdef WIN32
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
        if (!FAILED(hr))
        {
            HGLOBAL hBuf = 0;
            GetHGlobalFromStream(pStream, &hBuf);
            void* buffer = GlobalLock(hBuf);
            HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
            if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
            else
            {
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
#else // WIN32
#endif
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
#ifdef WIN32
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
#else

#endif // WIN32
    }

    napi_value openFile(napi_env env, napi_callback_info info)
    {
        napi_status status;

        napi_value jsthis;
        size_t argc = 1;
        napi_value args[1];
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        napi_valuetype valuetype;
        status = napi_typeof(env, args[0], &valuetype);
        assert(status == napi_ok);
        if (valuetype != napi_string)
        {
            napi_throw_error(env, "1", "Parameter error");
            return jsthis;
        }
        char value[MAX_PATH] = { 0 };
        size_t size;
        status = napi_get_value_string_latin1(env, args[0], value, MAX_PATH, &size);
        assert(status == napi_ok);
        QString strFilePath = value;
        for (int i = 0; i < g_listFiles.count(); i++)
        {
            if (g_listFiles.at(i).strPath == strFilePath)
            {
                return boolenValue(env, false);
            }
        }
        FileHandle files;

        if (!files.open(strFilePath.toLatin1().constData()))
        {
            return boolenValue(env, false);
        }
        g_listFiles.append(files);
        return boolenValue(env, true);
    }

    napi_value closeFile(napi_env env, napi_callback_info info)
    {
        napi_status status;

        napi_value jsthis;
        size_t argc = 1;
        napi_value args[1];
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        napi_valuetype valuetype;
        status = napi_typeof(env, args[0], &valuetype);
        assert(status == napi_ok);
        if (valuetype != napi_string)
        {
            napi_throw_error(env, "1", "Parameter error");
            return boolenValue(env, false);
        }
        char value[MAX_PATH] = { 0 };
        size_t size;
        status = napi_get_value_string_latin1(env, args[0], value, MAX_PATH, &size);
        assert(status == napi_ok);
        QString strFilePath = value;

        for (int i = 0; i < g_listFiles.count(); i++)
        {
            if (g_listFiles.at(i).strPath == strFilePath)
            {
                FileHandle file = g_listFiles.at(i);
                file.close();
                g_listFiles.removeAt(i);
                return boolenValue(env, true);
            }
        }
        return boolenValue(env, false);
    }

    napi_value findInstall(napi_env env, napi_callback_info info)
    {
        napi_status status;

        napi_value jsthis;
        size_t argc = 1;
        napi_value args[1];
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        napi_valuetype valuetype;
        status = napi_typeof(env, args[0], &valuetype);
        assert(status == napi_ok);
        if (valuetype != napi_string)
        {
            napi_throw_error(env, "1", "Parameter error");
            return boolenValue(env, false);
        }
        char value[1024] = { 0 };
        size_t size;
        status = napi_get_value_string_latin1(env, args[0], value, 1024, &size);
        assert(status == napi_ok);
        QString strProgrameName = value;
        QString strIcon;
        QString strIntallPath = findInstallPath(strProgrameName, strIcon);
        return boolenValue(env, strIntallPath == "" ? false : true);
    }

    napi_value openWithPrograme(napi_env env, napi_callback_info info)
    {
        napi_status status;

        napi_value jsthis;
        size_t argc = 2;
        napi_value args[2];
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        napi_valuetype valuetype[2];
        status = napi_typeof(env, args[0], &valuetype[0]);
        assert(status == napi_ok);
        status = napi_typeof(env, args[1], &valuetype[1]);
        assert(status == napi_ok);
        if (valuetype[0] != napi_string && valuetype[0] != napi_number)
        {
            napi_throw_error(env, "1", "Parameter error");
            return boolenValue(env, false);
        }
        char value[1024] = { 0 };
        size_t size;
        status = napi_get_value_string_latin1(env, args[0], value, 1024, &size);
        assert(status == napi_ok);

        QString strOpenFile = value;
        int nMode;
        napi_get_value_int32(env, args[1], &nMode);
        wchar_t wcFile[MAX_PATH] = { 0 };
        strOpenFile.toWCharArray(wcFile);

        if (nMode == 0)
        {
            ShellExecuteW(NULL, L"open", wcFile, NULL, NULL, SW_SHOW);
        }
        else
        {
#ifdef WIN32
            QString argm = "rundll32 shell32,OpenAs_RunDLL ";
            argm = argm + strOpenFile;

            QProcess::execute(argm);
#else
#endif
        }
        return boolenValue(env, true);
    }

    QString getBytesString(DWORD64 dw)
    {
        long rest = 0;
        if (dw < 1024)
        {
            return QString::number(rest) + "B";
        }
        else
        {
            dw /= 1024;
        }

        if (dw < 1024)
        {
            return QString::number(rest) + "KB";
        }
        else
        {
            rest = dw % 1024;
            dw /= 1024;
        }

        if (dw < 1024)
        {
            dw = dw * 100;
            return QString::number((dw / 100)) + "." + QString::number((rest * 100 / 1024 % 100)) + "MB";
        }
        else
        {
            dw = dw * 100 / 1024;
            return QString::number((dw / 100)) + "." + QString::number((dw % 100)) + "GB";
        }
    }

    napi_value DiskMessage(napi_env env, napi_callback_info info)
    {
        napi_status status;

        napi_value jsthis;
        size_t argc = 2;
        napi_value args[2];
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        napi_valuetype valuetype[2];
        status = napi_typeof(env, args[0], &valuetype[0]);
        assert(status == napi_ok);
        status = napi_typeof(env, args[1], &valuetype[1]);
        assert(status == napi_ok);
        if (valuetype[0] != napi_string && valuetype[1] != napi_function)
        {
            napi_throw_error(env, "1", "Parameter error");
            return nullptr;
        }
        char value[MAX_PATH] = { 0 };
        size_t size;
        status = napi_get_value_string_latin1(env, args[0], value, MAX_PATH, &size);
        assert(status == napi_ok);

        QString strDisk(value);
        QString strFreeToCaller, strTotal, strFree;
#ifdef WIN32
        strDisk = strDisk.left(2);
        DWORD64 qwFreeBytes, qwFreeBytesToCaller, qwTotalBytes;
        bool bResult = GetDiskFreeSpaceEx(strDisk.toLatin1().constData(),
            (PULARGE_INTEGER)&qwFreeBytesToCaller,
            (PULARGE_INTEGER)&qwTotalBytes,
            (PULARGE_INTEGER)&qwFreeBytes);
        if (bResult)
        {
            strFreeToCaller = getBytesString(qwFreeBytesToCaller);
            strTotal = getBytesString(qwTotalBytes);
            strFree = getBytesString(qwFreeBytes);
        }
#else
#endif
        napi_value global;
        status = napi_get_global(env, &global);
        napi_value argv[4] = { 0 };
        napi_value result;
        napi_value cb = args[1];

        argv[0] = boolenValue(env, bResult);
        argv[1] = stringValue(env, strFreeToCaller);
        argv[2] = stringValue(env, strTotal);
        argv[3] = stringValue(env, strFree);

        napi_call_function(env, global, cb, 4, argv, &result);
        return nullptr;
    }

    napi_value copyFile(napi_env env, napi_callback_info info)
    {
        napi_status status;

        napi_value jsthis;
        size_t argc = 1;
        napi_value args[1];
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        napi_valuetype valuetype[1];
        status = napi_typeof(env, args[0], &valuetype[0]);
        assert(status == napi_ok);
        status = napi_typeof(env, args[1], &valuetype[1]);
        assert(status == napi_ok);
        if (valuetype[0] != napi_string)
        {
            napi_throw_error(env, "1", "Parameter error");
            return nullptr;
        }
        char value[MAX_PATH] = { 0 };
        size_t size;
        status = napi_get_value_string_latin1(env, args[0], value, MAX_PATH, &size);
        assert(status == napi_ok);
        QString strFile(value);
        QStringList lstFile = strFile.split("*");

        if (!QGuiApplication::instance())
        {
            int argc = 0;
            QGuiApplication *app = new QGuiApplication(argc, 0);
        }
        QMimeData *data = new QMimeData;
        QClipboard *clip = QGuiApplication::clipboard();
        clip->clear();
        QList<QUrl> lstUrl;
        foreach(QString str, lstFile)
        {
            lstUrl << QUrl::fromLocalFile(str);
        }
        data->setUrls(lstUrl);
        clip->setMimeData(data);
        return nullptr;
    }

    napi_value Init(napi_env env, napi_value exports)
    {
        const int nPorperty = 6;
        napi_status status;
        napi_property_descriptor desc[nPorperty];
        desc[0] = DECLARE_NAPI_METHOD("openFile", openFile);
        desc[1] = DECLARE_NAPI_METHOD("closeFile", closeFile);
        desc[2] = DECLARE_NAPI_METHOD("findInstall", findInstall);
        desc[3] = DECLARE_NAPI_METHOD("openWithPrograme", openWithPrograme);
        desc[4] = DECLARE_NAPI_METHOD("DiskMessage", DiskMessage);
        desc[5] = DECLARE_NAPI_METHOD("copyFile", copyFile);
        status = napi_define_properties(env, exports, nPorperty, desc);
        assert(status == napi_ok);
        return exports;
    }
    NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
}