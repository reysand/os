#include <iostream>
#include <string>
#include <windows.h>

#define COUNT_THREADS 4
#define ITERATIONS 100

using namespace std;

CRITICAL_SECTION criticalSection;

DWORD value = 0;
HANDLE hMutex = nullptr;

HANDLE hEvent1, hEvent2;
int a[5];
unsigned long uThrID;

DWORD WINAPI	counterCS(LPVOID lpParam) {
	for (DWORD i = 0; i < ITERATIONS; ++i) {
		EnterCriticalSection(&criticalSection);
		wcout << L"Thread " << *(DWORD *)lpParam << L" : " << i << endl;
		LeaveCriticalSection(&criticalSection);
		Sleep(rand() % 16 + 10);
	}
	ExitThread(0);
}

void			funcCriticalSection() {
	HANDLE	threads[COUNT_THREADS];

	srand(time(nullptr));
	InitializeCriticalSection(&criticalSection);
	for (DWORD i = 0; i < COUNT_THREADS; ++i) {
		auto *tmp = new DWORD;
		*tmp = i;
		threads[i] = CreateThread(nullptr, 0, counterCS, tmp, 0, nullptr);
	}
	WaitForMultipleObjects(COUNT_THREADS, threads, TRUE, INFINITE);
	DeleteCriticalSection(&criticalSection);
	for (auto & thread : threads) {
		CloseHandle(thread);
	}
}

DWORD WINAPI	counter0(LPVOID lpParam) {
	while (value < ITERATIONS) {
		WaitForSingleObject(hMutex, INFINITE);
		wcout << L"func0: " << value++ << endl;
		ReleaseMutex(hMutex);
	}
	ExitThread(0);
}

DWORD WINAPI	counter1(LPVOID lpParam) {
	while (value < ITERATIONS) {
		WaitForSingleObject(hMutex, INFINITE);
		wcout << L"func1: " << value++ << endl;
		ReleaseMutex(hMutex);
	}
	ExitThread(0);
}

DWORD WINAPI	counter2(LPVOID lpParam) {
	while (value < ITERATIONS) {
		WaitForSingleObject(hMutex, INFINITE);
		wcout << L"func2: " << value++ << endl;
		ReleaseMutex(hMutex);
	}
	ExitThread(0);
}

DWORD WINAPI	counter3(LPVOID lpParam) {
	while (value < ITERATIONS) {
		WaitForSingleObject(hMutex, INFINITE);
		wcout << L"func3: " << value++ << endl;
		ReleaseMutex(hMutex);
	}
	ExitThread(0);
}

void			funcMutex() {
	HANDLE gThreads[COUNT_THREADS];

	hMutex = CreateMutex(nullptr, FALSE, reinterpret_cast<LPCSTR>(L"Mutex"));
	gThreads[0] = CreateThread(nullptr, 0, counter0, nullptr, NULL, nullptr);
	gThreads[1] = CreateThread(nullptr, 0, counter1, nullptr, NULL, nullptr);
	gThreads[2] = CreateThread(nullptr, 0, counter2, nullptr, NULL, nullptr);
	gThreads[3] = CreateThread(nullptr, 0, counter3, nullptr, NULL, nullptr);

	WaitForMultipleObjects(COUNT_THREADS, gThreads, TRUE, INFINITE);

	if (gThreads[0] != INVALID_HANDLE_VALUE) {
		CloseHandle(gThreads[0]);
	}
	if (gThreads[1] != INVALID_HANDLE_VALUE) {
		CloseHandle(gThreads[1]);
	}
	if (gThreads[2] != INVALID_HANDLE_VALUE) {
		CloseHandle(gThreads[2]);
	}
	if (gThreads[3] != INVALID_HANDLE_VALUE) {
		CloseHandle(gThreads[3]);
	}
}

void WINAPI Thread() {
	int num = 0;

	while (num <= ITERATIONS) {
		WaitForSingleObject( hEvent2, INFINITE );
		for (int & i : a) {
			i = num;
		}
		num++;
		SetEvent( hEvent1 );
	}
}

void	funcEvents() {
	hEvent1 = CreateEvent(nullptr, FALSE, TRUE, nullptr);
	hEvent2 = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Thread,
					 nullptr, 0, &uThrID);
	while (a[0] < ITERATIONS) {
		WaitForSingleObject(hEvent1, INFINITE);
		Sleep(100);
		printf("%d %d %d %d %d\n", a[0], a[1], a[2], a[3], a[4]);
		SetEvent(hEvent2);
	}
}

int	main()
{
	funcCriticalSection();
	cout << endl;
	funcMutex();
	cout << endl;
	funcEvents();
	return 0;
}
