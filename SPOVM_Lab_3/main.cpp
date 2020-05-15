#include <sys/wait.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

#define bufSize 30

using namespace std;

int main() {
    int filedes[2]; //0 - чтение, 1 - запись
    char buf[bufSize];
    const char exitStr[] = "exit";

    if (pipe(filedes) == -1) {
        cout << "Ошибка создания канала" << endl;
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        cout << "Ошибка создания процесса" << endl;
        exit(1);
    } else if (pid == 0) { //Клиент
        //Удалим дескриптор записи, так как не будем его использовать
        close(filedes[1]);

        while (true) {
            read(filedes[0], &buf, bufSize);

            //Если мы получили exit, то выходим
            if (strcmp(buf, exitStr) == 0) {
                close(filedes[1]);
                exit(0);
            }

            cout << "Получено сообщение от сервера:\n" << buf << endl;

            //Очистим буффер
            memset(buf, 0, bufSize);
        }
    } else { //Сервер
        //Удалим дескриптор чтения, так как не будем его использовать
        close(filedes[0]);

        while (true) {
            usleep(100);
            cout << "Введите сообщение клиенту:" << endl;
            cin >> buf;

            write(filedes[1], &buf, strlen(buf));

            //Если мы ввели exit, то выходим
            if (strcmp(buf, exitStr) == 0) {
                close(filedes[1]);
                wait(&pid);
                exit(0);
            }
        }
    }
}