#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>

#define BUFF_SIZE 256

#ifndef O_BINARY
#define O_BINARY 0
#endif

using namespace std;

class Commands {
public:
    static string getReply(int sock);
    static int sendCommand(int sock, char buff[BUFF_SIZE]);
    static int pasv(int sock, char buff[BUFF_SIZE]);
    static int list(int sock, int dataSock, char buff[BUFF_SIZE]);
    static int retr(int sock, int dataSock, char buff[BUFF_SIZE], char typeData, string downloadsPath);
    static int stor(int sock, int dataSock, char buff[BUFF_SIZE], char typeData, string dir);
    static char type(int sock, char buff[BUFF_SIZE]);
    static int quit(int sock, char buff[BUFF_SIZE]);
};
