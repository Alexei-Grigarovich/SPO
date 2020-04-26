#include <iostream>
#include <locale>
#include <cstdlib>
#include <conio.h>
#include <Windows.h>
#include <string>
#include <atlstr.h>
#include <vector>

using namespace std;

CRITICAL_SECTION section;

void printProcess(int p) {
	cout << "П";
	Sleep(100);
	cout << "р";
	Sleep(100);
	cout << "о";
	Sleep(100);
	cout << "ц";
	Sleep(100);
	cout << "е";
	Sleep(100);
	cout << "с";
	Sleep(100);
	cout << "c";
	Sleep(100);
	cout << " " << p << endl;
}

DWORD WINAPI threadFun(LPVOID lpParam) {
	int num = *(int*)lpParam;
	while (true) {
		while (TryEnterCriticalSection(&section) == 0) {}
		printProcess(num);
		LeaveCriticalSection(&section);
		Sleep(10);
	}
}

inline void close(vector<HANDLE> hndls) {
	for (int i = 0; i < hndls.size(); i++) {
		TerminateThread(hndls[i], 0);
	}
	DeleteCriticalSection(&section);
	exit(0);
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");

	vector<HANDLE> hndls;

	HANDLE thread;

	InitializeCriticalSection(&section);

	int* num = new int;
	*num = 0;

	while (true) {
		if (_kbhit()) {
			switch (_getch())
			{
			case '+':
				while (TryEnterCriticalSection(&section) == 0) {}

				thread = CreateThread(NULL, 0, threadFun, (LPVOID)num, 0, NULL);

				if (thread == NULL) {
					cout << "Error create thread" << endl;
					continue;
				}

				(*num)++;

				hndls.push_back(thread);

				LeaveCriticalSection(&section);

				break;
			case '-':
				if (!hndls.empty()) {
					while (TryEnterCriticalSection(&section) == 0) {}
					TerminateThread(hndls[hndls.size() - 1], 0);
					hndls.pop_back();
					(*num)--;
					LeaveCriticalSection(&section);
				}
				break;
			case 'q':
				close(hndls);
				break;
			case 169:
				close(hndls);
				break;
			default:
				break;
			}
		}
	}
}