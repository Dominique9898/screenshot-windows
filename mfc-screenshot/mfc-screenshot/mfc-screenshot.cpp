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

typedef struct mData {
	int screenNum;
	std::vector<std::string> paths;
}monitorData;

std::string GetLocalAppDataPath() {
	char buffer[MAX_PATH];
	SHGetSpecialFolderPathA(NULL, buffer, CSIDL_LOCAL_APPDATA, NULL);
	return buffer;
}

float mRound(float var)
{
	float value = (int)(var * 100 + .5);
	return (float)value / 100;
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
		monitorData* data = (monitorData*)dwData;
		int screenNum = data->screenNum++;
		// get logical width, height
		int wdithLogical = lprcMonitor->right - lprcMonitor->left;
		int heightLogical = lprcMonitor->bottom - lprcMonitor->top;

		// get physical width, height
		MONITORINFOEX info;
		info.cbSize = sizeof(info);
		GetMonitorInfo(hMonitor, &info);

		DEVMODE dm;
		dm.dmSize = sizeof(dm);
		dm.dmDriverExtra = 0;
		EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &dm);

		int widthPhysical = dm.dmPelsWidth;
		int heightPhysical = dm.dmPelsHeight;
		int cxPhysical = dm.dmPosition.x;
		int cyPhysical = dm.dmPosition.y;

		std::cout << "width: " << widthPhysical << " height: " << heightPhysical << std::endl;

		std::cout << "x: " << cxPhysical << " y: " << cyPhysical << std::endl;

		// get calculate scale factor
		float horzScale = mRound((float)widthPhysical / (float)wdithLogical);
		float vertScale = mRound((float)heightPhysical / (float)heightLogical);

		std::cout << "horzScale " << horzScale << std::endl;
		std::cout << "vertScale " << vertScale << std::endl;

		HDC hDCScreen = GetDC(NULL);
		HDC memdc = CreateCompatibleDC(hDCScreen);
		HBITMAP membit = CreateCompatibleBitmap(hDCScreen, widthPhysical, heightPhysical);

		std::cout << "membit: " << membit << std::endl;

		HBITMAP hOldBitmap = (HBITMAP)SelectObject(memdc, membit);

		std::cout << "hOldBitmap: " << hOldBitmap << std::endl;

		BitBlt(memdc, 0, 0, widthPhysical, heightPhysical, hDCScreen, cxPhysical, cyPhysical, SRCCOPY);

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
		data->paths.push_back(strFileName);
	}

	GdiplusShutdown(gdiplusToken);
	// restore
	return true;
}
int main()
{
	monitorData mdata;
	mdata.screenNum = 0;
	HDC hDCScreen = GetDC(NULL);
	EnumDisplayMonitors(hDCScreen, NULL, (MONITORENUMPROC)(&CaptureEnumMonitorsFunc), LPARAM(&mdata));
	DeleteDC(hDCScreen);
	getchar();
}