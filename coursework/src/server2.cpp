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

#define DEFAULT_PORT "27016"
#define DEFAULT_BUFLEN 512

using namespace	std;

struct addrinfo	*result = nullptr,
				*ptr = nullptr,
				hints;
WSADATA			wsaData;
SOCKET			ListenSocket = INVALID_SOCKET;
SOCKET			ClientSocket = INVALID_SOCKET;

int	disconnectServer() {
	int	iResult;

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("[FAIL] Shutdown: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	printf("[PASS] Shutdown\n");
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
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

int	sendData() {
	DWORD		processClientId;
	DWORD		processServer1Id;
	DWORD		processServer2Id;
	char		filename[MAX_PATH];
	char		buf[DEFAULT_BUFLEN];
	char		recvbuf[DEFAULT_BUFLEN];
	int			iResult, iSendResult;
	int			recvbuflen = DEFAULT_BUFLEN;

	GetModuleFileName(nullptr, filename, MAX_PATH);
	processClientId = GetProcessByName("client.exe");
	processServer1Id = GetProcessByName("server1.exe");
	processServer2Id = GetProcessByName("server2.exe");
	sprintf(buf, "Full module filename: %s\nClient pid: %lu\nServer1 pid: %lu"
			  "\nServer2 pid: %lu\n", filename, processClientId, processServer1Id,
			  processServer2Id);
	cout << endl << "LOGS: " << endl << buf << endl;
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("[PASS] Receive request: %d bytes\n", iResult);
			iSendResult = send(ClientSocket, buf, (int)strlen(buf), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("[FAIL] Send: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("[PASS] Send: %d bytes\n", iSendResult);
		} else if (iResult == 0)
			return disconnectServer();
		else {
			printf("[FAIL] Receive request: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);
	return 0;
}

int	acceptConnection() {
	ClientSocket = accept(ListenSocket, nullptr, nullptr);
	if (ClientSocket == INVALID_SOCKET) {
		printf("[FAIL] Accepting a connection: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("[PASS] Accepting a connection\n");
	return 0;
}

int	listenSocket() {
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf( "[FAIL] Listening on a ListenSocket: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("[PASS] Listening on a ListenSocket\n");
	return 0;
}

int	bindSocket() {
	int	iResult;

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("[FAIL] Binding a socket: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	return 0;
}

int	createSocket() {
	int	iResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
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

int	main() {
	int	ret = 0;

	do {
		ret += initWinSock();
		ret += createSocket();
		ret += bindSocket();
		ret += listenSocket();
		ret += acceptConnection();
		ret += sendData();
	} while (!ret);
	cerr << "Press any key to continue . . .";
	_getch();
	return ret;
}
