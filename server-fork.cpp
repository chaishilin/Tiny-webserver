#include <signal.h>
#include <stdio.h>
//#include<stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
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
int listenfd, connfd;
sockaddr_in seraddr, cliaddr;
in_addr sa;
socklen_t clilen;
pid_t childpid;
int connum = 0;
string recvbuf;
string sendbuf;
char recvbuf_c[BUFSIZE];
char sendbuf_c[BUFSIZE];
//func

string UrlDecode(const string &szToDecode)
{
    string result;
    int hex = 0;
    for (size_t i = 0; i < szToDecode.length(); ++i)
    {
        switch (szToDecode[i])
        {
        case '+':
            result += ' ';
            break;
        case '%':
            if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
            {
                string hexStr = szToDecode.substr(i + 1, 2);
                hex = strtol(hexStr.c_str(), 0, 16);
                //字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
                //可以不经过编码直接用于URL
                if (!((hex >= 48 && hex <= 57) ||  //0-9
                      (hex >= 97 && hex <= 122) || //a-z
                      (hex >= 65 && hex <= 90) ||  //A-Z
                      //一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@]
                      hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29 || hex == 0x2a || hex == 0x2b || hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f || hex == 0x3A || hex == 0x3B || hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f))
                {
                    result += char(hex);
                    i += 2;
                }
                else
                    result += '%';
            }
            else
            {
                result += '%';
            }
            break;
        default:
            result += szToDecode[i];
            break;
        }
    }
    return result;
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
void doreverse(string &str)
{
    auto iter_begin = str.begin();
    auto iter_end = str.end() - 1;
    while (iter_begin < iter_end)
    {
        swap(*iter_begin, *(iter_end - 2));
        swap(*(iter_begin + 1), *(iter_end-1));

        swap(*(iter_begin + 2), *iter_end);
        iter_end -= 3;
        iter_begin += 3;
    }
}
string parser(string request)
{
    string response = "";
    if (request.substr(0, 4) == "POST")
    {
        string result = request.substr(request.find("fname") + 6);
        cout << "fname:" << result << endl;
        string dresult = UrlDecode(result);
        doreverse(dresult);
        response += "HTTP/1.0 200 OK\r\n";
        response += "\r\n";
        response += "<HTML><B>hello world!</B>";
        response += "<head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head>";
        //response += "<B>You are the " + to_string(connum + 1) + "th clients!</B>";
        response += "<form accept-charset='utf-8' name='myForm' method='post'>字符串: <input type='text' name='fname'><input type='submit' value='submit'></form>";
        response += "<B>Result: " + dresult + "</B>";
        response += "</HTML>";
    }
    else if (request.substr(0, 3) == "GET")
    {
        response += "HTTP/1.0 200 OK\r\n";
        response += "\r\n";
        response += "<HTML><B>hello world!</B>";
        response += "<head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head>";
        //response += "<B>You are the " + to_string(connum + 1) + "th clients!</B>";
        response += "<form accept-charset='utf-8' name='myForm' method='post'>字符串: <input type='text' name='fname'><input type='submit' value='submit'></form>";

        response += "</HTML>";
    }
    else
    {
        response = "HTTP/1.0 400 BadRequest\r\n";
    }

    return response;
}
void sig_child(int signo)
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        //connum--;
        cout << "end child:" << pid << endl;
        //cout << "client number: " << connum << endl;
    }

    return;
}
void print_addr(const sockaddr_in cliaddr)
{
    sa.s_addr = cliaddr.sin_addr.s_addr;
    cout << inet_ntoa(sa) << ":";
    cout << htons(cliaddr.sin_port) << endl;
}

void my_echo(int connfd)
{
    int n;
    while (1)
    {
        if (n = (recv(connfd, recvbuf_c, BUFSIZE, 0)) > 0)
        {
            cout << "get request "
                 << "from: ";
            print_addr(cliaddr);
            string response = parser(string(recvbuf_c));
            //doreverse(recvbuf);
            if (send(connfd, response.c_str(), response.length() * sizeof(char), 0) <= 0)
            {
                cout << "send error" << endl;
                break;
            }
        }
        else
        {
            cout << "read error" << endl;
            break;
        }
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
    signal(SIGCHLD, sig_child);
    cout << "Begin to listen" << endl;
    while (1)
    {
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
            if ((childpid = fork()) == 0)
            {
                //child
                close(listenfd);
                my_echo(connfd);
                cout << "end the echo" << endl;
                exit(0);
            }
            else
            {
                connum++;
                cout << "client number:  " << connum << endl;
                close(connfd);
            }
        }
    }

    exit(0);

    return 0;
}