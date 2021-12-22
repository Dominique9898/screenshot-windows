#include "stdafx.h"
#include <windows.h>
#include <GdiPlus.h>
#include <string>
#include <ShlObj.h>
#include <vector>
#include <iostream>
#include <ctime>

#pragma comment( lib, "gdiplus" )

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
int screenNum;
std::vector<std::string> paths;
HDC hDCScreen;

std::string GetLocalAppDataPath() {
	char buffer[MAX_PATH];
	SHGetSpecialFolderPathA(NULL, buffer, CSIDL_LOCAL_APPDATA, NULL);
	return buffer;
}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return 0;
}
BOOL CaptureEnumMonitorsFunc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	using namespace Gdiplus;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	{
		screenNum++;
		int width = lprcMonitor->right - lprcMonitor->left;
		int height = lprcMonitor->bottom - lprcMonitor->top;
		int x = lprcMonitor->left;
		int y = lprcMonitor->top;

		std::cout << "width: " << width << " height: " << height << std::endl;

		std::cout << "x: " << x << " y: " << y << std::endl;

		HDC memdc = CreateCompatibleDC(hDCScreen);
		HBITMAP membit = CreateCompatibleBitmap(hDCScreen, width, height);

		std::cout << "membit: " << membit << std::endl;

		HBITMAP hOldBitmap = (HBITMAP)SelectObject(memdc, membit);

		std::cout << "hOldBitmap: " << hOldBitmap << std::endl;

		BitBlt(memdc, 0, 0, width, height, hDCScreen, x, y, SRCCOPY);

		//File name saved to
		std::string strFileName = GetLocalAppDataPath() + "\\Temp\\screen_shot\\";
		time_t now = time(NULL);
		std::string s_now = std::to_string(now);
		strFileName = strFileName + "fullscreen_" + std::to_string(screenNum) + "_" + s_now + ".png";

		std::cout << "strFileName:" << strFileName << std::endl;

		Gdiplus::Bitmap bitmap(membit, NULL);
		CLSID clsid;
		GetEncoderClsid(L"image/png", &clsid);

		std::wstring wStrFileName = std::wstring(strFileName.begin(), strFileName.end());

		bitmap.Save(wStrFileName.c_str(), &clsid);

		SelectObject(memdc, hOldBitmap);

		DeleteObject(memdc);

		DeleteObject(membit);
		paths.push_back(strFileName);
	}

	GdiplusShutdown(gdiplusToken);
	// restore
	return true;
}
int main()
{
	screenNum = 0;
	paths.clear();
	hDCScreen = GetDC(NULL);
	EnumDisplayMonitors(hDCScreen, NULL, (MONITORENUMPROC)(&CaptureEnumMonitorsFunc), NULL);
	DeleteDC(hDCScreen);
	getchar();
}