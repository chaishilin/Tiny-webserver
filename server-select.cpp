#include <signal.h>
#include <stdio.h>
//#include<stdlib.h>
//#include<string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
using namespace std;

//const
const int BUFSIZE = 1024;
const int LISTENQ = 5;
const int PORT = 12345;

//var
int i, listenfd, connfd, sockfd, maxfd, maxi;
int nready, client[FD_SETSIZE];
sockaddr_in seraddr, cliaddr;
in_addr sa;
socklen_t clilen;
pid_t childpid;
int connum = 0;
char recvbuf[BUFSIZE];
char sendbuf[BUFSIZE];
fd_set rset, allset;
//func

void print_addr(const sockaddr_in cliaddr)
{
    sa.s_addr = cliaddr.sin_addr.s_addr;
    cout << inet_ntoa(sa) << ":";
    cout << htons(cliaddr.sin_port) << endl;
}
void doreverse(char *ptr)
{
    int num = 0;
    while (*(ptr + num) != '\0')
        num++;
    num--;
    int slow = 0;
    while (slow < num)
    {
        swap(*(ptr + slow), *(ptr + num));
        slow++;
        num--;
    }
}
void my_echo(int connfd)
{
    int n;

    if (n = (recv(connfd, recvbuf, BUFSIZE, 0)) > 0)
    {
        cout << "get: " << recvbuf << "\t";
        cout << "from: ";
        print_addr(cliaddr);
        doreverse(recvbuf);
        if (send(connfd, recvbuf, BUFSIZE, 0) <= 0)
        {
            cout << "send error" << endl;
        }
    }
    else
    {
        cout << "read error" << endl;
    }
}
int main()
{
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(PORT);
    bind(listenfd, (sockaddr *)&seraddr, sizeof(seraddr));
    listen(listenfd, LISTENQ);
    maxfd = listenfd;
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    cout << "Begin to listen" << endl;

    while (1)
    {
        rset = allset;
        nready = select(maxfd + 1, &rset, nullptr, nullptr, NULL);
        if (FD_ISSET(listenfd, &rset))
        {
            cout << "new client" << endl;
            clilen = sizeof(cliaddr);
            if ((connfd = accept(listenfd, (sockaddr *)&cliaddr, &clilen)) < 0)
            {
                if (errno == EINTR)
                    continue;
                else
                    exit(0);
            }
            else
            {

                for (i = 0; i < FD_SETSIZE; i++)
                {
                    if (client[i] < 0)
                    {
                        client[i] = connfd;
                        break;
                    }
                }
                //cout << "accepted:" << i << ":" << connfd << endl;
                if (i == FD_SETSIZE)
                {
                    cout << "too many clients" << endl;
                    break;
                }
                FD_SET(connfd, &allset);
                if (connfd > maxfd)
                    maxfd = connfd;
                if (i > maxi)
                    maxi = i;
                if (--nready <= 0)
                    continue;
            }
        }
        for (i = 0; i <= maxi; i++)
        {
            if ((sockfd = client[i]) < 0)
                continue;
            //cout << "wait:" << sockfd << endl;
            if (FD_ISSET(sockfd, &rset))
            {
                my_echo(sockfd);
                client[i] = -1;
                FD_CLR(sockfd, &allset);
                close(sockfd);
                nready--;
                cout << "close client No." << i << endl;
            }
            if (nready <= 0)
                break;
        }
    }

    exit(0);

    return 0;
}