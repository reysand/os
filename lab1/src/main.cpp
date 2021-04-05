#include <conio.h>
#include <iostream>
#include <windows.h>

#define MAX_USERNAME_LENGTH 256

using namespace std;

void	getSysNames() {
	char	*buf;
	DWORD	size;

	size = MAX_COMPUTERNAME_LENGTH + 1;
	if (!(buf = (char *)malloc(size))) {
		cout << "No memory allocated" << endl;
		return ;
	}
	if (GetComputerName(buf, &size)) {
		cout << "Computer name: " << buf << endl;
	} else {
		cout << "Computer name: Fail: " << GetLastError() << endl;
	}
	size = MAX_USERNAME_LENGTH + 1;
	if (!(buf = (char *)realloc(buf, size))) {
		free(buf);
		return ;
	}
	if (GetUserName(buf, &size)) {
		cout << "User name: " << buf << endl;
	} else {
		cout << "User name: Fail: " << GetLastError() << endl;
	}
	free(buf);
}

void	getSysDirsPath() {
	char	*buf;
	int		size;

	size = MAX_PATH + 1;
	if (!(buf = (char *)malloc(size))) {
		cout << "No memory allocated" << endl;
		return ;
	}
	if (GetSystemDirectory(buf, size)) {
		cout << "System directory: " << buf << endl;
	} else {
		cout << "System directory: Fail: " << GetLastError() << endl;
	}
	if (GetWindowsDirectory(buf, size)) {
		cout << "Windows directory: " << buf << endl;
	} else {
		cout << "Windows directory: Fail: " << GetLastError() << endl;
	}
	if (GetTempPath(size, buf)) {
		cout << "Temp path: " << buf << endl;
	} else {
		cout << "Temp path: Fail: " << GetLastError() << endl;
	}
	free(buf);
}

void	getWinVer() {
	auto	*osv = new RTL_OSVERSIONINFOEXW;
	typedef LONG(WINAPI *RtlGetVersion)(RTL_OSVERSIONINFOEXW*);

	memset(osv, 0, sizeof(RTL_OSVERSIONINFOEXW));
	osv->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	auto rtlGetVersion = (RtlGetVersion)GetProcAddress(ntdll, "RtlGetVersion");
	if (!rtlGetVersion)
	{
		return ;
	}
	LONG Status = rtlGetVersion(osv);
	cout << "Windows: ";
	if (Status == 0) {
		cout << osv->dwMajorVersion << "." << osv->dwMinorVersion << endl;
	} else {
		cout << "Unknown" << endl;
	}
	delete osv;
}

void	getSys() {
	int		elements[2] = {COLOR_ACTIVECAPTION, COLOR_SCROLLBAR};
	DWORD	new_colors[2] = {13125600, 12539515};
	DWORD	old_colors[2];
	int		value = 0;

	cout << "Monitors count: " << GetSystemMetrics(SM_CMONITORS) << endl;
	cout << "Screen size: " << GetSystemMetrics(SM_CXSCREEN) << "x";
	cout << GetSystemMetrics(SM_CYSCREEN) << endl;

	SystemParametersInfo(SPI_GETMOUSESPEED, 0, &value, 0);
	cout << "Mouse speed: " << value << endl;
	SystemParametersInfo(SPI_GETBORDER, 0, &value, 0);
	cout << "Border: " << value << endl;

	for (int i = 0; i < 2; ++i) {
		old_colors[i] = GetSysColor(elements[i]);
		cout << "Color " << i << ": " << old_colors[i] << endl;
	}
	cout << "Color change: ";
	if (SetSysColors(2, elements, new_colors)) {
		cout << "PASS" << endl;
	} else {
		cout << "FAIL" << endl;
	}
}

void	getTime() {
	SYSTEMTIME lt;

	GetLocalTime(&lt);
	cout << "Local time: " << lt.wHour << ":" << lt.wMinute << endl;
	cout << "Time since system start: " << GetTickCount() << endl;
}

int		main() {
	getSysNames();
	getSysDirsPath();
	getWinVer();
	getSys();
	getTime();
	cerr << "Press any key to continue . . .";
	_getch();
	return 0;
}
