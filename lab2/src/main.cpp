#include <iostream>
#include <string>
#include <windows.h>
#include <TlHelp32.h>

using namespace std;

string	getFileName(const string &s) {
	char	sep = '\\';
	size_t	i = s.rfind(sep, s.length());

	if (i != string::npos) {
		return(s.substr(i + 1, s.length() - i));
	}
	return("");
}

void	getHandleAndFNameOrName() {
	HMODULE	hmodule;
	char	input[MAX_PATH];
	char	buf[MAX_PATH];

	cin.getline(input, MAX_PATH);
	cout << "Handle: ";
	LoadLibrary(input);
	if ((hmodule = GetModuleHandle(input))) {
		cout << hmodule << endl;
	}
	if (getFileName(buf).empty()) {
		GetModuleFileName(hmodule, buf, MAX_PATH);
		if (!hmodule) {
			cout << GetModuleHandle(buf) << endl;
		}
		cout << "FName: " << buf << endl;
	} else {
		cout << "Name: " << getFileName(buf) << endl;
	}
}

void	getProcessInfo() {
	DWORD	processId = GetCurrentProcessId();
	HANDLE	process = GetCurrentProcess();
	HANDLE	processCopy;
	HANDLE	hMutex = CreateMutex(nullptr, FALSE, nullptr);
	HANDLE	hMutexDup;

	cout << "Current process id: " << GetCurrentProcessId() << endl;
	cout << "Pseudo handle: " << process << endl;
	DuplicateHandle(process, hMutex, process, &hMutexDup, 0, FALSE, 0);
	cout << "Handle: " << hMutexDup << endl;
	processCopy = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);
	cout << "Handle copy: " << processCopy << endl;
	if (CloseHandle(hMutexDup)) {
		cout << "Close handle: " << hMutexDup << endl;
	}
	if (CloseHandle(processCopy)) {
		cout << "Close handle copy: " << processCopy << endl;
	}
}

void	getSnapshot() {
	HANDLE			hSnap;
	PROCESSENTRY32	pe32;
	THREADENTRY32	te32;
	MODULEENTRY32	me32;

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0))) {
		if (Process32First(hSnap, &pe32)) {
			cout << "ID\tFile name" << endl;
			cout << pe32.th32ProcessID << "\t" << pe32.szExeFile << endl;
			while (Process32Next(hSnap, &pe32)) {
				cout << pe32.th32ProcessID << "\t" << pe32.szExeFile << endl;
			}
		}
		cout << endl;
		te32.dwSize = sizeof(THREADENTRY32);
		if (Thread32First(hSnap, &te32)) {
			cout << "Prior\tID" << endl;
			cout << te32.tpBasePri << "\t" << te32.th32ThreadID << endl;
			while (Thread32Next(hSnap, &te32)) {
				cout << te32.tpBasePri << "\t" << te32.th32ThreadID << endl;
			}
		}
		cout << endl;
		if (Module32First(hSnap, &me32)) {
			cout << "Handle\t\tModule" << endl;
			cout << me32.hModule << "\t" << me32.szModule << endl;
			while (Module32Next(hSnap, &me32)) {
				cout << me32.hModule << "\t" << me32.szModule << endl;
			}
		}
	}
	CloseHandle(hSnap);
}

int		main() {
	getHandleAndFNameOrName();
	cout << endl;
	getProcessInfo();
	cout << endl;
	getSnapshot();
	return 0;
}
