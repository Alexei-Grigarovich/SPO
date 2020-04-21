#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <gtk/gtk.h>

#define BUFF_SIZE 512

using namespace std;

//    gtk_init(&argc, &argv);
//
//    GtkWidget* win;
//
//    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//    gtk_window_set_default_size(GTK_WINDOW(win),200, 200);
//    gtk_window_set_position(GTK_WINDOW(win));
//    gtk_window_set_resizable(GTK_WINDOW(win), false);
//    g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(gtk_main_quit), NULL);
//
//    gtk_container_set_border_width(GTK_CONTAINER(win), 10);
//    gtk_window_set_title(GTK_WINDOW(win), "Client");
//    gtk_container_add(GTK_CONTAINER(win), hbox);
//
//    gtk_widget_show_all(win);
//    gtk_main();

int initSocket() {
    int sock, connecting;
    sockaddr_in address;

    //Сокет
    sock = socket(AF_INET, SOCK_STREAM, 0);

    //Адрес
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_family = AF_INET;
    address.sin_port = htons(21);

    connecting = connect(sock, (sockaddr*)&address, sizeof(address));
    if(connecting == -1) {
        cout << "Error connecting..." << endl;
        return -1;
    } else return sock;
}

int initData(int sock) {

}

int readResponse(int sock) {
    string buff;

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(sock,&fd);

    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    do {
        buff = "";
        recv(sock, &buff, BUFF_SIZE,0);

        cout << buff << endl;
    } while(select(sock + 1,& fd, nullptr, nullptr, &timeout) != -1);
}

void login(int sock) {
    string buff, name, password;

    cout << "Введите имя: ";
    getline(cin, name);

    buff = "USER " + name + "\r\n";
    send(sock, (char*)&buff, strlen((char*)&buff), 0);

    readResponse(sock);

    cout << "Введите пароль: ";
    getline(cin, password);

    buff = "PASS " + password + "\r\n";
    send(sock, (char*)&buff, strlen((char*)&buff), 0);

    readResponse(sock);
}

void getFile() {

}

void sendFile() {

}

int main(int argc, char *argv[]) {
    int sock = initSocket();
    readResponse(sock);


    close(sock);
    return 0;
}
