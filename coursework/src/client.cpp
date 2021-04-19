#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <TlHelp32.h>

#define PORT_1 "27015"
#define PORT_2 "27016"
#define DEFAULT_BUFLEN 512
#define PS0 "client"
#define PS1 "client$ "

using namespace	std;

struct addrinfo	*result = nullptr,
				*ptr = nullptr,
				hints;
WSADATA			wsaData;
HANDLE hMutex = nullptr;
SOCKET			ListenSocket = INVALID_SOCKET;

int	disconnectClient() {
	int	iResult;

	iResult = shutdown(ListenSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("[FAIL] Shutdown: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("[PASS] Shutdown\n");
	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}

int	receiveData() {
	int recvbuflen = DEFAULT_BUFLEN;
	const char *sendbuf = "receive";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	iResult = send(ListenSocket, sendbuf, 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("[FAIL] Send a request: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("[PASS] Send a request: %d bytes\n", iResult);
	iResult = shutdown(ListenSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("[FAIL] Shutdown: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	do {
		iResult = recv(ListenSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("[PASS] Receive: %d bytes\n", iResult);
			recvbuf[iResult] = '\0';
			cout << endl << "LOGS:\n" << recvbuf << endl;
		} else if (iResult == 0)
			return disconnectClient();
		else
			printf("[FAIL] Receive: %d\n", WSAGetLastError());
	} while (iResult > 0);
	return 0;
}

int	connectSocket() {
	int	iResult;

	iResult = connect(ListenSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (ListenSocket == INVALID_SOCKET) {
		printf("[FAIL] Server connection\n");
		WSACleanup();
		return 1;
	}
	printf("[PASS] Server connection\n");
	return 0;
}

int	createSocket(PCSTR port) {
	int	iResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	iResult = getaddrinfo("127.0.0.1", port, &hints, &result);
	if (iResult != 0) {
		printf("[FAIL] getaddrinfo: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	ptr = result;
	ListenSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("[FAIL] socket: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	printf("[PASS] Creating a socket\n");
	return 0;
}

int	initWinSock() {
	int	iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("[FAIL] Initializing WinSock: %d\n", iResult);
		return 1;
	}
	printf("[PASS] Initializing WinSock\n");
	return 0;
}

DWORD WINAPI	server1(LPVOID cmd) {
	int	ret = 0;
	WaitForSingleObject(hMutex, INFINITE);
	if (initWinSock()) {
		return 1;
	}
	ret += createSocket(PORT_1);
	ret += connectSocket();
	ret += receiveData();
	ReleaseMutex(hMutex);
	ExitThread(ret);
}

DWORD WINAPI	server2(LPVOID lpParam) {
	int	ret = 0;
	WaitForSingleObject(hMutex, INFINITE);
	if (initWinSock()) {
		return 1;
	}
	ret += createSocket(PORT_2);
	ret += connectSocket();
	ret += receiveData();
	ReleaseMutex(hMutex);
	ExitThread(ret);
}

DWORD	GetProcessByName(PCSTR name) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process;
	DWORD pid = 0;

	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);
	if (Process32First(snapshot, &process)) {
		do {
			if (string(process.szExeFile) == string(name)) {
				pid = process.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &process));
	}
	CloseHandle(snapshot);
	return pid;
}

int	main() {
	HANDLE	server[2];
	HANDLE	tmpProcess;
	DWORD	processServer1Id;
	DWORD	processServer2Id;
	string	cmd;
	int		ret = 0;

	cout << PS1;
	while (getline(cin, cmd)) {
		if (cmd == "receive") {
			cout << endl;
			hMutex = CreateMutex(nullptr, FALSE, reinterpret_cast<LPCSTR>(L"Mutex"));
			server[0] = CreateThread(nullptr, 0, server1, nullptr, NULL, nullptr);
			server[1] = CreateThread(nullptr, 0, server2, nullptr, NULL, nullptr);
			WaitForMultipleObjects(2, server, TRUE, INFINITE);
			if (server[0] != INVALID_HANDLE_VALUE) {
				CloseHandle(server[0]);
			}
			if (server[1] != INVALID_HANDLE_VALUE) {
				CloseHandle(server[1]);
			}
			cout << endl;
		} else if (cmd == "exit") {
			cout << endl;
			processServer1Id = GetProcessByName("server1.exe");
			tmpProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processServer1Id);
			if (tmpProcess) {
				TerminateProcess(tmpProcess, 0);
				cout << "[PASS] Server 1 terminated" << endl;
			}
			processServer2Id = GetProcessByName("server2.exe");
			tmpProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processServer2Id);
			if (tmpProcess) {
				TerminateProcess(tmpProcess, 0);
				cout << "[PASS] Server 2 terminated" << endl;
			}
			closesocket(ListenSocket);
			WSACleanup();
			break;
		} else {
			cout << PS0 << ": " << cmd << ": command not found" << endl;
		}
		cout << PS1;
	}
	cerr << "Press any key to continue . . .";
	_getch();
	return ret;
}
