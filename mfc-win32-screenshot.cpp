// electron-shot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Windows.h"
#include <iostream>
#include <atlimage.h>
#include <atltime.h>
#include <string>
#include <vector>

BOOL CaptureEnumMonitorsFunc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CaptureWindowFromDC(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor);
CImage m_MyImage;
HDC hDCImg = NULL;
int screenNum = 0;
std::vector<std::string> paths;

int main()
{
	HDC hDCScreen = GetDC(NULL);
	EnumDisplayMonitors(hDCScreen, NULL, (MONITORENUMPROC)(&CaptureEnumMonitorsFunc), NULL);
	std::cout << "ok" << std::endl;
	
	for (int i = 0; i < paths.size(); i++) {
		std::cout << "strFileName: " << paths[i] << std::endl;
	}
	
	DeleteDC(hDCScreen);
	DeleteDC(hDCImg);
	getchar();
	return 0;
}

BOOL CaptureEnumMonitorsFunc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	if (!CaptureWindowFromDC(hMonitor, hdcMonitor, lprcMonitor)) return false;
	return true;
};

BOOL CaptureWindowFromDC(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor) {
	screenNum++;
	int width = lprcMonitor->right - lprcMonitor->left;
	int height = lprcMonitor->bottom - lprcMonitor->top;
	int bitPerPixel = GetDeviceCaps(hdcMonitor, BITSPIXEL);//Get the number of bits per pixel
	int x = lprcMonitor->left;
	int y = lprcMonitor->top;

	std::cout << "width:" << width << " height:" << height << std::endl;
	std::cout << "bitPerPixel:" << bitPerPixel << std::endl;

	m_MyImage.Create(width, height, bitPerPixel);
	hDCImg = m_MyImage.GetDC();
	BitBlt(hDCImg, 0, 0, width, height, hdcMonitor, x, y, SRCCOPY);

	//File name saved to
	std::string strFileName = "C:\\";
	strFileName += "ScreenShot\\fullscreen";
	time_t now = time(NULL);
	std::string s_now = std::to_string(now);
	// std::string file_path = "/.wxwork_local/screen_shot/fullscreen_";
	strFileName = strFileName + "_" + std::to_string(screenNum) + "_"  + s_now + ".png";

	CString cs(strFileName.c_str());

	m_MyImage.Save(cs, Gdiplus::ImageFormatPNG);
	if (!strFileName.empty()) paths.push_back(strFileName);

	m_MyImage.ReleaseDC();
	m_MyImage.Destroy();
	// restore
	return true;
};
