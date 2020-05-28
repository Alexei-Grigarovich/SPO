#include "Commands.h"


void Commands::sendCommand(int sock, char buff[BUFF_SIZE]) {
    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;
}

int Commands::pasv(int sock, char buff[BUFF_SIZE]) {
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

void Commands::list(int sock, int dataSock, char buff[BUFF_SIZE]) {
    char dataBuff[BUFF_SIZE];

    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;

    usleep(100);

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

void Commands::retr(int sock, int dataSock, char buff[BUFF_SIZE], char typeData, string path) {
    int readed;
    char dataBuff[BUFF_SIZE];
    ofstream oFile;
    string fileName;

    if (strlen(buff) < 5) {
        cout << " Недостаточно аргументов" << endl;
        return;
    }

    send(sock, buff, BUFF_SIZE, 0);

    //Запомним название файла
    buff[strlen(buff) - 2] = '\0';
    fileName = buff;
    fileName = fileName.substr(5);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;


    if (strstr(buff, "125") != nullptr) {
        //Создадим файл
        if (typeData == 'I') oFile.open(path + fileName, ios::binary);
            else oFile.open(path + fileName);

        if(!oFile) {
            cout << "Ошибка открытия каталога/файла" << endl;
            return;
        } else {
            cout << "\"" << path + fileName << "\" создан" << endl;
        }

        cout << "Скачивание файла \"" << path + fileName << "\"..." << endl;

        //Запись
        while ((readed = recv(dataSock, dataBuff, BUFF_SIZE, 0)) > 0) {
            oFile.write(dataBuff, readed);
        }

        cout << "Скачивание завершено" << endl;

        oFile.close();

        //Ответ сервера
        memset(buff, 0, BUFF_SIZE);
        recv(sock, buff, BUFF_SIZE, 0);
        cout << buff;
    }
}

void Commands::stor(int sock, int dataSock, char buff[BUFF_SIZE], char typeData, string path) {
    char dataBuff[BUFF_SIZE];
    string fileName;
    ifstream iFile;

    if (strlen(buff) < 5) {
        cout << " Недостаточно аргументов" << endl;
        return;
    }

    send(sock, buff, BUFF_SIZE, 0);

    //Запомним имя файла
    buff[strlen(buff) - 2] = '\0';
    fileName = buff;
    fileName = fileName.substr(5);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;


    //Передача файла
    if (strstr(buff, "125") != nullptr) {
        //Откроем файл
        if (typeData == 'I') iFile.open(path + fileName, ios::binary);
            else iFile.open(path + fileName);

        if (!iFile) {
            cout << "Файл \"" << path + fileName << "\" не найден" << endl;
            return;
        } else {
            cout << "\"" << path + fileName << "\" создан" << endl;
        }

        cout << "Передача файла \"" << path + fileName << "\"..." << endl;

        //Теперь будем считывать строку в буффер, затем отправлять серверу
        while (!iFile.eof()) {
            iFile.read(dataBuff, BUFF_SIZE);
            send(dataSock, dataBuff, BUFF_SIZE, 0);
        }

        cout << "Передача завершена" << endl;

        iFile.close();

        //Ответ сервера
        memset(buff, 0, BUFF_SIZE);
        recv(sock, buff, BUFF_SIZE, 0);
        cout << buff;
    }
}

char Commands::type(int sock, char buff[BUFF_SIZE]) {
    char typeData = buff[strlen("TYPE") + 1];

    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;

    if (strstr(buff, "200") != nullptr) return typeData;
    else return 'A';
}

int Commands::quit(int sock, char buff[BUFF_SIZE], pthread_t pthread, int dataSock) {
    sendCommand(sock, buff);

    pthread_cancel(pthread);
    pthread_join(pthread, 0);
    close(dataSock);
    close(sock);

    return 0;
}