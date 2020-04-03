#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include<errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h> 
using namespace std;

//const
const int BUFSIZE = 1024;
const int LISTENQ = 5;
const int PORT = 12345;
//var
int sockfd;
sockaddr_in seraddr;
pid_t childpid;
char recvbuf[BUFSIZE];
char sendbuf[BUFSIZE];
//func
void my_send(int connfd, char *msg)
{
    while (1)
    {
        //cout << msg << endl;
        if (*msg == '\0')
            cin >> sendbuf;
        else
            strcpy(sendbuf, msg);
        

        //cin >> sendbuf;
        bool ans = (*sendbuf);
        //sleep(1);
        if (send(connfd, sendbuf, BUFSIZE, 0) <= 0 && ans != 0)
        {
            cout << "client recv error" << endl;
            break;
        }
        else if (ans == 0)
        {
            cout << "end the send" << endl;
            break;
        }
        sendbuf[0] = '\0'; //clear the buf
        if (recv(connfd, recvbuf, BUFSIZE, 0) <= 0)
        {
            cout << "client recv error" << endl;
            break;
        }
        cout << recvbuf << endl;
    }
}
int main(int argc, char **argv)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    
    seraddr.sin_family = AF_INET;
    inet_pton(AF_INET,argv[1],&seraddr.sin_addr);
    seraddr.sin_port = htons(PORT);

    connect(sockfd, (sockaddr *)&seraddr, sizeof(seraddr));
    cout << "connet ok!" << endl;
    if (argc == 3)
        my_send(sockfd, argv[2]);
    else
    {;
        char t = '\0';
        my_send(sockfd,&t);
    }

    close(sockfd);
    cout << "close connect" << endl;
    exit(0);
    return 0;
}