//FTP Client implementation

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
#include <sys/poll.h>
#include <signal.h>         // sigaction
#include <stdio.h>          // for NULL, perror
#include <string.h>
#include <unistd.h>         // read, write, close
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

char * ftpServerName; 
const string PROMPT = "ftp> ";
int clientSd = -1;
const int CTRL_PORT = 21; //Port for FTP communication
const int BUF_SIZE = 1500;
char * response;

char* getServerResponse();
void getPassword();
void logIn();

int main(int argc, char *argv[]) {
	//Check for valid argument count
	if(argc != 2) {
		cerr << "Invalid number of arguments." << endl;
		cerr << "Requires hostname of ftp server to be passed in." << endl;
		cerr << endl;
		exit(0);
	}

	ftpServerName = argv[1];
	logIn();

	return 0;
}

//Reads response from server into char[] buffer and returns the buffer
char* getServerResponse() {
	char buff[BUF_SIZE];
	read(clientSd, (char*)buff, sizeof(buff));
	return buff;
}

void getPassword() {
	while(true) {
		cout << "Password: ";
		char password[20];
		cin >> password;
		char passwordCmd[27];
		strcat(passwordCmd, "PASS ");
		strcat(passwordCmd, password);
		strcat(passwordCmd, "\r\n");

		//Send password to server
		write(clientSd, (char *)&passwordCmd, strlen(passwordCmd));
		//print response
		response = getServerResponse();
		cout << response;

		string error = "501";

		//If it is a valid password break out of loop
		if(!strstr(response.c_str(), error.c_str())) {
			break;
		}
	}
}

int pollSocket() {
	struct pollfd ufds;
	ufds.fd = clientSd;               // a socket descriptor to exmaine for read
    ufds.events = POLLIN;             // check if this sd is ready to read
    ufds.revents = 0;                 // simply zero-initialized
    return poll( &ufds, 1, 1000 ); 	  // poll this socket for 1000msec (=1sec)
}

//Handles logging in user to ftp server
void logIn() {
	clientSd = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSd < 0) {
		cerr << "Error creating socket to server" << endl;
		exit(0);
	}

	struct hostent *host = gethostbyname(ftpServerName);
	struct sockaddr_in addr;
    bzero((char*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    addr.sin_port = htons(CTRL_PORT);

    //Attempt to connect socket to ftp server
    if(connect(clientSd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Cannot connect to ftp server." << endl;
        exit(0);
    }

    //Print welcome message from server
    response = getServerResponse();
    cout << response;

    //Log in user to server

    //Request username
    cout << "Name(" << ftpServerName << ":" << getenv("USER") << "): ";
    char name[20];
    cin >> name;
    char userCmd[27];
    strcpy(userCmd, "USER ");
    strcat(userCmd, name);
    strcat(user, "\r\n"); //CRLF

    //Send username to server and recieve the acknowledgement
    write(clientSd, (char*)&userCmd, strlen(userCmd));
    response = getServerResponse();
    cout << response;

    //Enter password
    getPassword();

    //Poll the server
    while(pollSocket() == 1) {
      response = getServerResponse();
    	cout << response << endl;
    }
    if(pollSocket() == -1) {
    	cout << "Can't poll socket" << endl;
    }
}