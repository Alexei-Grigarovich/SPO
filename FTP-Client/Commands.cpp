#include "Commands.h"


string Commands::getReply(int sock) {
    char buff[BUFF_SIZE] = "";

    //Ответ сервера
    if (recv(sock, buff, BUFF_SIZE, 0) != -1) {
        cout << "< " << buff;
        return buff;
    } else if (errno != ECONNRESET) return "0";

    return "-1";
}

int Commands::sendCommand(int sock, char buff[BUFF_SIZE]) {
    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0)
        return -1;

    return 0;
}

int Commands::pasv(int sock, char buff[BUFF_SIZE]) {
    int port = 0, dataSock = 0, temp[6];
    char address[BUFF_SIZE];
    sockaddr_in serverSock;
    string str;


    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0)
        return -1;

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
        cout << "< Ошибка создания сокета" << endl;
        return 0;
    } else cout << "< Сокет создан" << endl;

    serverSock.sin_family = AF_INET;
    serverSock.sin_addr.s_addr = inet_addr(address);
    serverSock.sin_port = htons(port);

    cout << "< Соединение с " << address << ":" << port << endl;
    if(connect(dataSock,(struct sockaddr*)&serverSock, sizeof(serverSock)) == -1) {
        cout << "< Ошибка инициализации соедениния данных" << endl;
        return 0;
    } else {
        cout << "< Соединение открыто" << endl;
        return dataSock;
    }
}

int Commands::list(int sock, int dataSock, char buff[BUFF_SIZE]) {
    char dataBuff[BUFF_SIZE];

    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0)
        return -1;


    while(recv(dataSock, dataBuff, BUFF_SIZE, 0) > 0) {
        cout << "< " << dataBuff;
        memset(dataBuff, 0, BUFF_SIZE);
    }

    close(dataSock);

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0) return -1;
        else return 0;
}

int Commands::retr(int sock, int dataSock, char buff[BUFF_SIZE], char typeData, string path) {
    int readed;
    char dataBuff[BUFF_SIZE];
    ofstream oFile;
    string fileName;

    if (strlen(buff) < 5) {
        cout << "< Недостаточно аргументов" << endl;
        return 1;
    }

    send(sock, buff, BUFF_SIZE, 0);

    //Запомним название файла
    buff[strlen(buff) - 2] = '\0';
    fileName = buff;
    fileName = fileName.substr(5);

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0)
        return -1;


    if (strstr(buff, "125") != nullptr) {
        //Создадим файл
        if (typeData == 'I') oFile.open(path + fileName, ios::binary);
            else oFile.open(path + fileName);

        if(!oFile) {
            cout << "< Ошибка открытия каталога/файла" << endl;
            return 1;
        } else cout << "\"" << path + fileName << "\" создан" << endl;

        cout << "< Скачивание файла \"" << path + fileName << "\"..." << endl;

        //Запись
        while ((readed = recv(dataSock, dataBuff, BUFF_SIZE, 0)) > 0) {
            oFile.write(dataBuff, readed);
        }
        oFile.close();
        close(dataSock);

        cout << "< Скачивание завершено" << endl;
    }

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0) return -1;
        else return 0;
}

int Commands::stor(int sock, int dataSock, char buff[BUFF_SIZE], char typeData, string path) {
    char dataBuff[BUFF_SIZE];
    string fileName;
    int file = 0, sent = 0;

    if (strlen(buff) < 5) {
        cout << "< Недостаточно аргументов" << endl;
        return 1;
    }

    send(sock, buff, BUFF_SIZE, 0);

    //Запомним имя файла
    buff[strlen(buff) - 2] = '\0';
    fileName = buff;
    fileName = fileName.substr(5);

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0)
        return -1;


    //Передача файла
    if (strstr(buff, "125") != nullptr) {
        //Откроем файл
        if (typeData == 'I') file = open((path + fileName).c_str(), O_RDONLY | O_BINARY);
            else file = open((path + fileName).c_str(), O_RDONLY);

        if (fopen == nullptr) {
            cout << "< Файл \"" << path + fileName << "\" не найден" << endl;
            return 1;
        } else cout << "< Файл \"" << path + fileName << "\" открыт" << endl;

        cout << "< Передача файла \"" << path + fileName << "\"..." << endl;

        //Теперь отправим файл серверу
        off_t offset = 0;
        struct stat file_stat;
        fstat(file, &file_stat);
        ssize_t size = file_stat.st_size;
        sent = sendfile(dataSock, file, &offset, size);

        close(file);
        close(dataSock);

        cout << "< Передано " << sent << " байт" << endl;
    }

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0) return -1;
        else return 0;
}

char Commands::type(int sock, char buff[BUFF_SIZE]) {
    char typeData = buff[strlen("TYPE") + 1];

    send(sock, buff, BUFF_SIZE, 0);

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0)
        return -1;

    if (strstr(buff, "200") != nullptr) return typeData;
    else return 'A';
}

int Commands::quit(int sock, char buff[BUFF_SIZE]) {
    if (sendCommand(sock, buff) == -1)
        return -1;

    close(sock);

    return 1;
}