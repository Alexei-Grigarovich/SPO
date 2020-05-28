#include "Commands.h"

bool isWorking = false;
char typeData = 'A';

struct data {
    int filedes[2]; //0 - чтение, 1 - запись
    int sock = 0, dataSock = 0;
};

void* threadFun(void* arg) {
    char buff[BUFF_SIZE];
    int pipeRead = ((data*)arg)->filedes[0], pipeWrite = ((data*)arg)->filedes[1];
    int sock = ((data*)arg)->sock, dataSock = ((data*)arg)->dataSock;

    while(true) {
        read(pipeRead, &buff, BUFF_SIZE);
        isWorking = true;

        //LIST - список файлов
        if (strstr(buff, "LIST") != nullptr && dataSock != 0) Commands::list(sock, dataSock, buff);

        //RETR - скачать файл с сервера
        if (strstr(buff, "RETR") != nullptr && dataSock != 0) Commands::retr(sock, dataSock, buff, typeData);

        //STOR - загрузить файл на сервер
        if (strstr(buff, "STOR") != nullptr && dataSock != 0) Commands::stor(sock, dataSock, buff, typeData);

        isWorking = false;
    }
}

int main() {
    //Переменные
    char address[BUFF_SIZE], buff[BUFF_SIZE];
    int sock = 0, dataSock = 0, authFlag = 1, port = 0, filedes[2];
    sockaddr_in serverSock;
    pthread_t pthread;


    //Создание канала
    if (pipe(filedes) == -1) {
        cout << "Ошибка создания канала" << endl;
        exit(1);
    }

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


    while(authFlag == 1) {
        memset(&buff, 0, BUFF_SIZE);
        cin.getline(buff, BUFF_SIZE);

        buff[strlen(buff)] = '\r';
        buff[strlen(buff)] = '\n';

        //USER
        if (strstr(buff, "USER") != nullptr) Commands::sendCommand(sock, buff);

        //PASS
        if (strstr(buff, "PASS") != nullptr) Commands::sendCommand(sock, buff);

        //PASV - пассивный режим
        if (strstr(buff, "PASV") != nullptr && dataSock == 0) {
            dataSock = Commands::pasv(sock, buff);

            //Создание потока
            data* args = new data;
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
        if (strstr(buff, "ABOR") != nullptr) Commands::sendCommand(sock, buff);

        //CWD - перейти в директорию
        if (strstr(buff, "CWD") != nullptr) Commands::sendCommand(sock, buff);

        //MKD - создать директорию
        if (strstr(buff, "MKD") != nullptr) Commands::sendCommand(sock, buff);

        //PWD - вернуть директорию
        if (strstr(buff, "PWD") != nullptr) Commands::sendCommand(sock, buff);

        //RMD - удалить директорию
        if (strstr(buff, "RMD") != nullptr) Commands::sendCommand(sock, buff);

        //TYPE - установить режим записи файлов
        if (strstr(buff, "TYPE") != nullptr) typeData = Commands::type(sock, buff);

        //DELE - удалить файл
        if (strstr(buff, "DELE") != nullptr && dataSock != 0) Commands::sendCommand(sock, buff);

        //SIZE - вернуть размер файла/папки
        if (strstr(buff, "SIZE") != nullptr && dataSock != 0) Commands::sendCommand(sock, buff);

        //QUIT - отключиться
        if (strstr(buff, "QUIT") != nullptr) authFlag = Commands::quit(sock, buff, pthread, dataSock);
    }

    return 0;
}