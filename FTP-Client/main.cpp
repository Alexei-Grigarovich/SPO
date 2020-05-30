#include "Commands.h"

bool isWorking = false;
char typeData = 'A';
string path = "";
int authFlag = 0;

struct data {
    int filedes[2]; //0 - чтение, 1 - запись
    int sock = 0, dataSock = 0;
};

void* threadFun(void* arg) {
    char buff[BUFF_SIZE];
    int pipeRead = ((data*)arg)->filedes[0], pipeWrite = ((data*)arg)->filedes[1];
    int sock = ((data*)arg)->sock, dataSock = ((data*)arg)->dataSock;

    read(pipeRead, &buff, BUFF_SIZE);
    isWorking = true;

    cout << "\r< Выполнение задачи в фоновом потоке..." << endl;

    //LIST - список файлов
    if (strstr(buff, "LIST") != nullptr && dataSock != 0) authFlag = Commands::list(sock, dataSock, buff);

    //RETR - скачать файл с сервера
    if (strstr(buff, "RETR") != nullptr && dataSock != 0) authFlag = Commands::retr(sock, dataSock, buff, typeData, path);

    //STOR - загрузить файл на сервер
    if (strstr(buff, "STOR") != nullptr && dataSock != 0) authFlag = Commands::stor(sock, dataSock, buff, typeData, path);

    cout << "< Завершение выполнения задачи в фоновом потоке..." << endl;

    memset(buff, 0, BUFF_SIZE);
    isWorking = false;
    pthread_exit(0);
}

int main() {
    //Переменные
    char address[BUFF_SIZE] = "13.56.207.108", buff[BUFF_SIZE];
    int sock = 0, dataSock = 0, port = 2000, filedes[2];
    sockaddr_in serverSock;
    pthread_t pthread;


    //Создание канала
    if (pipe(filedes) == -1) {
        cout << "< Ошибка создания канала" << endl;
        exit(1);
    }

    //Создание сокета
    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock == -1) {
        cout << "< Ошибка создания сокета";
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
        cout << "< Ошибка соединения" << endl;
        return 1;
    }else {
        cout << "< Успешное соединение!" << endl;
    }

    //timeout sock
    timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    //Ответ сервера
    strcpy(buff, Commands::getReply(sock).c_str());
    if (strcmp(buff, "-1") == 0)
        return -1;


    while(authFlag == 0) {
        memset(&buff, 0, BUFF_SIZE);

        cout << "> ";
        cin.getline(buff, BUFF_SIZE);

        buff[strlen(buff)] = '\r';
        buff[strlen(buff)] = '\n';

        //USER
        if (strstr(buff, "USER") != nullptr) authFlag = Commands::sendCommand(sock, buff);

        //PASS
        if (strstr(buff, "PASS") != nullptr) authFlag = Commands::sendCommand(sock, buff);

        //PASV - пассивный режим
        if (strstr(buff, "PASV") != nullptr) {
            dataSock = Commands::pasv(sock, buff);

            timeval timeout;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            setsockopt(dataSock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

            //Создание потока
            data* args = new data;
            args->dataSock = dataSock;
            args->sock = sock;
            args->filedes[0] = filedes[0];
            args->filedes[1] = filedes[1];

            if (pthread_create(&pthread, nullptr, threadFun, args) != 0) {
                cout << "< Ошибка создания потока" << endl;
            }
        }

        //LIST - список файлов
        if (strstr(buff, "LIST") != nullptr && !isWorking) write(filedes[1], &buff, strlen(buff));

        //RETR - скачать файл с сервера
        if (strstr(buff, "RETR") != nullptr && !isWorking) {
            //Ввод директории файла для скачивания
            cout << "Куда сохранить файл? - ";
            cin >> path;
            write(filedes[1], &buff, strlen(buff));
        }

        //STOR - загрузить файл на сервер
        if (strstr(buff, "STOR") != nullptr && !isWorking) {
            //Ввод директории файла для загрузки
            cout << "Введите директорию файла - ";
            cin >> path;
            write(filedes[1], &buff, strlen(buff));
        }

        //ABOR - прервать передачу файла
        if (strstr(buff, "ABOR") != nullptr) authFlag = Commands::sendCommand(sock, buff);

        //CWD - перейти в директорию
        if (strstr(buff, "CWD") != nullptr) authFlag = Commands::sendCommand(sock, buff);

        //MKD - создать директорию
        if (strstr(buff, "MKD") != nullptr) authFlag = Commands::sendCommand(sock, buff);

        //PWD - вернуть директорию
        if (strstr(buff, "PWD") != nullptr) authFlag = Commands::sendCommand(sock, buff);

        //RMD - удалить директорию
        if (strstr(buff, "RMD") != nullptr) authFlag = Commands::sendCommand(sock, buff);

        //TYPE - установить режим записи файлов
        if (strstr(buff, "TYPE") != nullptr) {
            typeData = Commands::type(sock, buff);
            if (typeData == -1 || typeData == 1) authFlag == typeData;
        }

        //DELE - удалить файл
        if (strstr(buff, "DELE") != nullptr && dataSock != 0) authFlag = Commands::sendCommand(sock, buff);

        //SIZE - вернуть размер файла/папки
        if (strstr(buff, "SIZE") != nullptr && dataSock != 0) authFlag = Commands::sendCommand(sock, buff);

        //QUIT - отключиться
        if (strstr(buff, "QUIT") != nullptr) authFlag = Commands::quit(sock, buff);
    }

    cout << "< Завершение работы" << endl;
    return 0;
}