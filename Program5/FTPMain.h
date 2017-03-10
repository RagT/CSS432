/*
Handles interacting with the server. Executes ftp commands for the client.
*/

#include <arpa/inet.h>      // inet_ntoa
#include <netinet/in.h>     // htonl, htons, inet_ntoa
#include <netinet/tcp.h>    // TCP_NODELAY
#include <sys/socket.h>     // socket, bind, listen, inet_ntoa
#include <sys/stat.h>
#include <sys/types.h>      // socket, bind
#include <sys/uio.h>        // writev
#include <sys/wait.h>       // for wait
#include <fcntl.h>          // fcntl
#include <netdb.h>          // gethostbyname
#include <poll.h>
#include <signal.h>         // sigaction
#include <stdio.h>          // for NULL, perror
#include <string.h>
#include <unistd.h>         // read, write, close
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class FTPMain {
public:
	FTPMain();

	//FTP Commands (Return server response as a string)
	string open(string host, string port);
	string user(string username);
	string password(string password);
	string cd(string dir);
	string ls();
	string get(filename);
	string put(filename);
	string close();
	string quit();

private:
	static const int DEFAULT_PORT = 21; //Default server port for FTP
	static const int BUF_SIZE = 1448;

	int serverPortNum;
	int clientSd;
	struct hostent * host;
	struct sockaddr_in sendAddr;
}