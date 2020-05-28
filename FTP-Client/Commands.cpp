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

void Commands::retr(int sock, int dataSock, char buff[BUFF_SIZE], char typeData) {
    string downloadsPath = "downloads/";
    char dataBuff[BUFF_SIZE];
    ofstream oFile;
    string fileName;

    if (strlen(buff) < 5) {
        cout << " Недостаточно аргументов" << endl;
        return;
    }

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
        //Ввод директории файла для загрузки
        cout << "Введите директорию для сохранения файла - ";
        cin >> downloadsPath;

        //Создадим файл
        if (typeData == 'I') oFile.open(downloadsPath + fileName, ios::binary);
        else oFile.open(downloadsPath + fileName);
        if(!oFile.is_open()) {
            cout << "Ошибка открытия каталога/файла" << endl;
            return;
        }

        cout << "Скачивание..." << endl;

        //Запись
        int readed;
        while ((readed = recv(dataSock, dataBuff, BUFF_SIZE, 0)) > 0) {
            oFile << buff;
            if (readed < BUFF_SIZE) break;
        }
        oFile.close();

        cout << "Скачивание завершено" << endl;

        //Ответ сервера
        memset(buff, 0, BUFF_SIZE);
        recv(sock, buff, BUFF_SIZE, 0);
        cout << buff;
    }
}

void Commands::stor(int sock, int dataSock, char buff[BUFF_SIZE], char typeData) {
    string fileName;
    ifstream iFile;
    string dir;

    if (strlen(buff) < 5) {
        cout << " Недостаточно аргументов" << endl;
        return;
    }

    //Запомним имя файла
    fileName = buff;
    fileName = fileName.substr(5);

    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    memset(buff, 0, BUFF_SIZE);
    recv(sock, buff, BUFF_SIZE, 0);
    cout << buff;


    //Передача файла
    if (strstr(buff, "125") != nullptr) {
        //Ввод директории файла для загрузки
        cout << "Введите директорию файла - ";
        cin >> dir;

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