#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <ncurses.h>
#include <pthread.h>
#include <locale.h>

using namespace std;
struct data {
    pthread_mutex_t* mutex = nullptr;
    int num;
};

bool kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}

void printChild(int p) {
    int sleeptime = 100000;
    string str = "Поток ";
    cout << "\r";
    for(int i = 0; i < str.size(); i++) {
        cout << str[i];
        cout.flush();
        usleep(sleeptime);
    }
    cout << p << endl;
}

void* threadFun(void* arg) {
    int id = ((data*)arg)->num;
    while(true) {
        pthread_mutex_lock(((data*)arg)->mutex);

        printChild(id);

        pthread_mutex_unlock(((data*)arg)->mutex);
        usleep(100);
    }
}

int main() {
    initscr();
    noecho();
    setlocale(LC_ALL, "ru");

    int num = 0;

    pthread_mutex_t mutex;
    if(pthread_mutex_init(&mutex, nullptr) != 0) {
        cout << "Error creating mutex" << endl;
    }

    vector<pthread_t> threads;
    pthread_t pthread;

    data *args = new data;
    args->mutex = &mutex;

    while(true) {
        if(kbhit()) {
            switch (getch()) {
                case '+':
                    pthread_mutex_lock(&mutex);

                    args->num = num;

                    if (pthread_create(&pthread, nullptr, threadFun, args) == 0) {
                        threads.push_back(pthread);
                        num++;
                    } else {
                        cout << "Error creating thread" << endl;
                    }
                    pthread_mutex_unlock(&mutex);
                    break;
                case '-':
                    if(num > 0 && !threads.empty()) {
                        pthread_mutex_lock(&mutex);
                        pthread_cancel(threads.back());
                        pthread_join(threads.back(), 0);
                        threads.pop_back();
                        num--;
                        pthread_mutex_unlock(&mutex);
                    }
                    break;
                case 'q':
                    for(pthread_t t : threads) pthread_cancel(t);
                    pthread_mutex_destroy(&mutex);
                    endwin();
                    exit(0);
                    break;
            }
        }
    }
}
