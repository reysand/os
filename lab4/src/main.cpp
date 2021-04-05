#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <iomanip>
#include <conio.h>

using namespace std;

#define FOREGROUND_RESET (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define MB 1048576

string	progressBar(DWORD curValue, DWORD maxValue) {
	string	progressBar;
	int		progress = ceil(curValue * 100.0 / (maxValue * 10));

	for (int i = 0; i < 10; ++i) {
		if (i < progress) {
			progressBar += "|";
		} else {
			progressBar += " ";
		}
	}
	return progressBar;
}

void	print(const string& memType, DWORDLONG avail, DWORDLONG total) {
	HANDLE	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	cout << "Available " << memType << "\t [";
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
	cout << progressBar(avail, total);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RESET);
	cout << avail << "/" << total << "MB]" << endl;
}

void	getMemoryStatus() {
	HANDLE			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	MEMORYSTATUSEX	memSt;

	memSt.dwLength = sizeof(memSt);
	GlobalMemoryStatusEx(&memSt);
	cout << "Memory Load\t [";
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	cout << progressBar(memSt.dwMemoryLoad, 100);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RESET);
	cout << memSt.dwMemoryLoad << "/100%]" << endl;
	print("PMem", memSt.ullAvailPhys / MB, memSt.ullTotalPhys / MB);
	print("PFile", memSt.ullAvailPageFile / MB, memSt.ullTotalPageFile / MB);
	print("VMem", memSt.ullAvailVirtual / MB, memSt.ullTotalVirtual / MB);
	cout << "Available ExtMem " << memSt.ullAvailExtendedVirtual / MB << "MB" << endl;
}

void	getProcessList() {
	HANDLE			hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32	pe32;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (hSnap) {
		if (Process32First(hSnap, &pe32)) {
			cout << "ID\tProcess Name" << endl;
			do {
				cout << pe32.th32ProcessID << "\t" << pe32.szExeFile << endl;
			} while (Process32Next(hSnap, &pe32));
		}
	}
}

void	getVirtualMap(DWORD idProcess) {
	MEMORY_BASIC_INFORMATION	memInfo;
	HANDLE						hProcess;
	DWORD						finalAddress = 0x7FFFFFFF;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, idProcess);
	cout << "Base Address\tBlock Size\tState\tType\tProtect" << endl;
	for (DWORD address = 0; address < finalAddress; address += memInfo.RegionSize) {
		VirtualQueryEx(hProcess, (void *)address, &memInfo, sizeof(memInfo));
		cout << hex << "0x" << setw(8) << setfill('0') << memInfo.BaseAddress;
		cout << "\t" << "0x" << setw(8) << memInfo.RegionSize << setfill(' ');
		cout << dec << "\t";
		if (memInfo.State == MEM_FREE) {
			cout << "Free\t";
		} else if (memInfo.State == MEM_RESERVE || memInfo.State == MEM_COMMIT) {
			cout << "Reserve\t";
		} else {
			cout << "Unknown\t";
		}
		if (memInfo.Type == MEM_IMAGE) {
			cout << "Image\t";
		} else if (memInfo.Type == MEM_MAPPED) {
			cout << "Mapped\t";
		} else if (memInfo.Type == MEM_PRIVATE) {
			cout << "Private\t";
		} else {
			cout << "Unknown\t";
		}
		if (memInfo.Protect == PAGE_EXECUTE) {
			cout << "Execute" << endl;
		} else if (memInfo.Protect == PAGE_NOACCESS) {
			cout << "No access" << endl;
		} else if (memInfo.Protect == PAGE_READONLY) {
			cout << "Read only" << endl;
		} else if (memInfo.Protect == PAGE_READWRITE) {
			cout << "Read/Write" << endl;
		} else if (memInfo.Protect == PAGE_WRITECOPY) {
			cout << "Write/Copy" << endl;
		} else {
			cout << "Unknown" << endl;
		}
	}
}

int		main() {
	DWORD	idProcess;
	int		ret = 0;

	getMemoryStatus();
	cout << endl;
	getProcessList();
	cout << endl << "Input process id: ";
	cin >> idProcess;
	if (idProcess) {
		getVirtualMap(idProcess);
	} else {
		cerr << "[1] Bad process id" << endl;
		ret = 1;
	}
	cerr << "Press any key to continue . . .";
	_getch();
	return ret;
}
