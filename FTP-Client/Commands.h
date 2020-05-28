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

class Commands {
public:
    static void sendCommand(int sock, char buff[BUFF_SIZE]);
    static int pasv(int sock, char buff[BUFF_SIZE]);
    static void list(int sock, int dataSock, char buff[BUFF_SIZE]);
    static void retr(int sock, int dataSock, char buff[BUFF_SIZE], char typeData);
    static void stor(int sock, int dataSock, char buff[BUFF_SIZE], char typeData);
    static char type(int sock, char buff[BUFF_SIZE]);
    static int quit(int sock, char buff[BUFF_SIZE], pthread_t pthread, int dataSock);
};
