#define _CRT_SECURE_NO_WARNINGS

#include <locale.h>
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

LPWSTR CharToLPWSTR(LPCSTR char_string)
{
	LPWSTR res;
	DWORD res_len = MultiByteToWideChar(1251, 0, char_string, -1, NULL, 0);
	res = (LPWSTR)GlobalAlloc(GPTR, (res_len + 1) * sizeof(WCHAR));
	MultiByteToWideChar(1251, 0, char_string, -1, res, res_len);
	return res;
}

void main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	if (argc == 1) {
		string message;

		STARTUPINFO info;
		PROCESS_INFORMATION processInfo;

		ZeroMemory(&info, sizeof(info));
		ZeroMemory(&processInfo, sizeof(processInfo));

		HANDLE Work = CreateSemaphore(NULL, 0, 1, L"Work");
		HANDLE WINAPI fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1, L"FileProjection");

		LPVOID mapView = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (!CreateProcess(CharToLPWSTR(argv[0]), (LPWSTR)L" child", NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &processInfo)) {
			cout << "Process start error" << endl;
			system("pause");
			exit(0);
		}

		cout << "Введите сообщение клиенту" << endl;
		while (true) {
			getline(cin, message);

			strcpy((char*)mapView, const_cast<char*>(message.c_str()));

			ReleaseSemaphore(Work, 1, NULL);
			WaitForSingleObject(Work, INFINITE);

			if (message == "exit") {
				TerminateProcess(processInfo.hProcess, 0);
				CloseHandle(fileMapping);
				CloseHandle(Work);
				CloseHandle(processInfo.hProcess);
				CloseHandle(processInfo.hThread);
				UnmapViewOfFile(mapView);
				exit(0);
			}
		}
	}
	else {
		HANDLE Work = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"Work");
		HANDLE WINAPI fileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"FileProjection");

		LPVOID mapView = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		cout << "Клиент:" << endl;

		while (true)
		{
			WaitForSingleObject(Work, INFINITE);

			cout << "Получено сообщение от сервера: " << (char*)mapView << endl;

			ReleaseSemaphore(Work, 1, NULL);
		}
	}
}