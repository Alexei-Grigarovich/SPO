#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>

#define BUFF_SIZE 256

using namespace std;

char typeData = 'A';
string downloadsPath = "downloads/";
bool isWorking = false;

struct data {
    pthread_mutex_t* mutex = nullptr;
    int filedes[2]; //0 - чтение, 1 - запись
    int sock = 0, dataSock = 0;
};

void sendCommand(int sock, char buff[BUFF_SIZE]) {
    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;
}

int pasv(int sock, char buff[BUFF_SIZE]) {
    int port = 0, dataSock = 0, temp[6];
    char address[BUFF_SIZE];
    sockaddr_in serverSock;
    string str;


    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;


    //Обработка полученной строки
    str = buff;
    str = str.substr(str.find('(') + 1, str.find(')') - str.find('(') - 1);
    std::replace(str.begin(), str.end(), ',', ' ');
    strcpy(buff, str.c_str());
    sscanf(buff, "%d %d %d %d %d %d", &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5]);
    port = temp[4] * 256 + temp[5];
    sprintf(address, "%d.%d.%d.%d", temp[0], temp[1], temp[2], temp[3]);


    dataSock = socket(AF_INET,SOCK_STREAM,0);
    if(dataSock == -1) {
        cout << "Ошибка создания сокета" << endl;
        return 0;
    } else cout << "Сокет создан" << endl;

    serverSock.sin_family = AF_INET;
    serverSock.sin_addr.s_addr = inet_addr(address);
    serverSock.sin_port = htons(port);

    cout << "Соединение с " << address << ":" << port << endl;
    if(connect(dataSock,(struct sockaddr*)&serverSock, sizeof(serverSock)) == -1) {
        cout << "Ошибка инициализации соедениния данных" << endl;
        return 0;
    } else {
        cout << "Соединение открыто" << endl;
        return dataSock;
    }
}

void list(int sock, int dataSock, char buff[BUFF_SIZE]) {
    char dataBuff[BUFF_SIZE];

    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;

    int readed;
    while((readed = recv(dataSock, dataBuff, BUFF_SIZE, 0)) > 0) {
        cout << dataBuff;
        if (readed < BUFF_SIZE) break;
    }

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;
}

void retr(int sock, int dataSock, char buff[BUFF_SIZE]) {
    char dataBuff[BUFF_SIZE];
    ofstream oFile;
    string fileName;

    //Запомним название файла
    int tmp = strlen("RETR") + 1;
    for(int i = 0; tmp < strlen(buff) + 1; i++)
        fileName[i] = buff[tmp++];


    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;


    if (strstr(buff, "125") != nullptr) {
        //Создадим файл
        if (typeData == 'I') oFile.open(downloadsPath + fileName, ios::binary);
        else oFile.open(downloadsPath + fileName);
        if(!oFile.is_open()) {
            cout << "Ошибка открытия каталога" << endl;
            return;
        }

        //Запись
        int readed;
        while ((readed = recv(dataSock, dataBuff, BUFF_SIZE, 0)) > 0) {
            oFile << buff;
            if (readed < BUFF_SIZE) break;
        }

        oFile.close();
    } else cout << buff;
}

void stor(int sock, int dataSock, char buff[BUFF_SIZE]) {
    string fileName;
    ifstream iFile;
    string dir;

    send(sock, buff, BUFF_SIZE, 0);


    //Запомним имя файла
    fileName = buff;
    fileName = fileName.substr(5);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;


    //Ввод директории файла для загрузки
    cout << "Введите директорию файла - ";
    cin >> dir;


    //Передача файла
    if (strstr(buff, "125") != nullptr) {
        //Откроем файл
        if (typeData == 'I') iFile.open(dir + fileName, ios::binary);
        else iFile.open(dir + fileName);

        if (!iFile.is_open()) {
            cout << "Файл не найден" << endl;
            return;
        }

        //Теперь будем считывать строку в буффер, затем отправлять серверу
        while (!iFile.eof()) {
            memset(buff, 0, BUFF_SIZE);
            iFile.read(buff, BUFF_SIZE);
            send(dataSock, buff, BUFF_SIZE, 0);
        }

        //Закроем файл
        cout << "Передача завершена" << endl;
        iFile.close();
    }

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;
}

char type(int sock, char buff[BUFF_SIZE]) {
    char typeData = buff[strlen("TYPE") + 1];

    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;

    if (strstr(buff, "200") != nullptr) return typeData;
    else return 'A';
}

int quit(int sock, char buff[BUFF_SIZE], pthread_t pthread, int dataSock, pthread_mutex_t mutex) {
    sendCommand(sock, buff);

    pthread_cancel(pthread);
    pthread_join(pthread, 0);
    pthread_mutex_destroy(&mutex);
    close(dataSock);
    close(sock);

    return 0;
}

void* threadFun(void* arg) {
    char buff[BUFF_SIZE];
    pthread_mutex_t mutex = *(((data*)arg)->mutex);
    int pipeRead = ((data*)arg)->filedes[0], pipeWrite = ((data*)arg)->filedes[1];
    int sock = ((data*)arg)->sock, dataSock = ((data*)arg)->dataSock;

    while(true) {
        read(pipeRead, &buff, BUFF_SIZE);
        isWorking = true;

        //LIST - список файлов
        if (strstr(buff, "LIST") != nullptr && dataSock != 0) list(sock, dataSock, buff);

        //RETR - скачать файл с сервера
        if (strstr(buff, "RETR") != nullptr && dataSock != 0) retr(sock, dataSock, buff);

        //STOR - загрузить файл на сервер
        if (strstr(buff, "STOR") != nullptr && dataSock != 0) stor(sock, dataSock, buff);

        isWorking = false;
    }
}

int main() {
    //Переменные
    char address[BUFF_SIZE], buff[BUFF_SIZE];
    int sock = 0, dataSock = 0, authFlag = 0, port = 0, filedes[2];
    sockaddr_in serverSock;
    pthread_t pthread;

    //Создание канала
    if (pipe(filedes) == -1) {
        cout << "Ошибка создания канала" << endl;
        exit(1);
    }


    //Мютекс
    pthread_mutex_t mutex;
    if(pthread_mutex_init(&mutex, nullptr) != 0) {
        cout << "Ошибка создания мютекса" << endl;
    }
    pthread_mutex_lock(&mutex);


    //Создание сокета
    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock == -1) {
        cout << "Ошибка создания сокета";
        return 1;
    }


    //Ввод адреса
    cout << "Введите адресс для подключения - ";
    cin >> address;

    //Ввод порта
    cout << "Введите порт для подключения - ";
    cin >> port;
    cin.ignore();


    //Соединение
    serverSock.sin_family = AF_INET;
    serverSock.sin_addr.s_addr = inet_addr(address);
    serverSock.sin_port = htons(port);

    if(connect(sock,(struct sockaddr*)&serverSock, sizeof(serverSock)) == -1) {
        cout << "Ошибка соединения" << endl;
        return 1;
    }else {
        cout << "Успешное соединение!" << endl;
    }

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;


    authFlag = 1;
    while(authFlag == 1) {
        memset(&buff, 0, BUFF_SIZE);
        cin.getline(buff, BUFF_SIZE);

        //USER
        if (strstr(buff, "USER") != nullptr) sendCommand(sock, buff);

        //PASS
        if (strstr(buff, "PASS") != nullptr) sendCommand(sock, buff);

        //PASV - пассивный режим
        if (strstr(buff, "PASV") != nullptr && dataSock == 0) {
            dataSock = pasv(sock, buff);

            //Создание потока
            data* args = new data;
            args->mutex = &mutex;
            args->dataSock = dataSock;
            args->sock = sock;

            if (pthread_create(&pthread, nullptr, threadFun, args) != 0) {
                cout << "Ошибка создания потока" << endl;
            }
        }

        //LIST - список файлов
        if (strstr(buff, "LIST") != nullptr && dataSock != 0 && !isWorking) write(filedes[1], &buff, strlen(buff));

        //RETR - скачать файл с сервера
        if (strstr(buff, "RETR") != nullptr && dataSock != 0 && !isWorking) write(filedes[1], &buff, strlen(buff));

        //STOR - загрузить файл на сервер
        if (strstr(buff, "STOR") != nullptr && dataSock != 0 && !isWorking) write(filedes[1], &buff, strlen(buff));

        //ABOR - прервать передачу файла
        if (strstr(buff, "ABOR") != nullptr) sendCommand(sock, buff);

        //CWD - перейти в директорию
        if (strstr(buff, "CWD") != nullptr) sendCommand(sock, buff);

        //MKD - создать директорию
        if (strstr(buff, "MKD") != nullptr) sendCommand(sock, buff);

        //PWD - вернуть директорию
        if (strstr(buff, "PWD") != nullptr) sendCommand(sock, buff);

        //RMD - удалить директорию
        if (strstr(buff, "RMD") != nullptr) sendCommand(sock, buff);

        //TYPE - установить режим записи файлов
        if (strstr(buff, "TYPE") != nullptr) typeData = type(sock, buff);

        //DELE - удалить файл
        if (strstr(buff, "DELE") != nullptr && dataSock != 0) sendCommand(sock, buff);

        //SIZE - вернуть размер файла/папки
        if (strstr(buff, "SIZE") != nullptr && dataSock != 0) sendCommand(sock, buff);

        //QUIT - отключиться
        if (strstr(buff, "QUIT") != nullptr) authFlag = quit(sock, buff, pthread, dataSock, mutex);
    }

    return 0;
}