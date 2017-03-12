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
#include <vector>
#include <sstream>
using namespace std;

char * ftpServerName; 
const string PROMPT = "ftp> ";
int clientSd = -1;
const int CTRL_PORT = 21; //Port for FTP communication
char * response;
bool loggedIn = false;
const int BUF_SIZE = 8192;
char buff[BUF_SIZE];

char* getServerResponse();
void getPassword();
void logIn();
void checkLogIn();

//Utility function for splitting strings by spaces
vector<string> split(string str) {
	vector<string> splitArr;
	char * token = strtok((char *) str.c_str(), " ");
	while(token != NULL) {
		splitArr.push_back(token);
		token = strtok(NULL, " ");
	} 
	return splitArr;
}

int main(int argc, char *argv[]) {
	//Check for valid argument count
	if(argc != 2) {
		cerr << "Invalid number of arguments." << endl;
		cerr << "Requires hostname of ftp server to be passed in." << endl;
		cerr << endl;
		exit(0);
	}

	ftpServerName = argv[1];

	cout << endl;

	//Command input loop
	while(true) {
		//Prompt user for command
		cout << PROMPT;
		string command; //string containing command
		getline(cin, command);

		//vector to store each part of command 
		vector<string> splitCommand = split(command);
		string commandType = splitCommand[0];

		if(commandType == "open") {
			logIn();
		} else if(commandType == "ls") {
			checkLogIn();
		}  else if(commandType == "cd") {
			checkLogIn();
			if(splitCommand.size() != 2) {
				cout << "Usage: cd subdir" << endl;
				continue;
			}
			char cdCommand[BUF_SIZE];
			strcpy(cdCommand, "CWD ");
			strcat(cdCommand, splitCommand[1].c_str());
			strcat(cdCommand, "\r\n");

			//send command to server
			write(clientSd, (char *)&cdCommand, strlen(cdCommand));
			response = getServerResponse();
			cout << response;
			continue;

		} else if( commandType == "get") {
			checkLogIn();
		} else if(commandType == "put") {
			checkLogIn();
		} else if(commandType == "close") {
			//Close connection to server but keep ftp client running
			checkLogIn();
			loggedIn = false;
		} else if(commandType == "quit") {
			//log out of server and quit ftp client

			//Exit program
			break;
		} else {
			cout << "Invalid command" << endl;
		}
	}

	return 0;
}

void checkLogIn() {
	if(!loggedIn) {
		cout << "Please log in with the 'open' command first" << endl;
	}
}

//Reads response from server into char[] buffer and returns the buffer
char* getServerResponse() {
	bzero(buff, sizeof(buff));
	read(clientSd, buff, sizeof(buff));
	string error = "421";
	if(strstr(buff, error.c_str())) {
		cout << "There was a problem with the server." << endl;
		exit(0);
	}
	return buff;
}

//Prompts user for their password until correct password is recieved.
void getPassword() {
	while(true) {
		cout << "Password: ";
		char password[20];
		cin >> password;
		char passwordCmd[27];
		strcpy(passwordCmd, "PASS ");
		strcat(passwordCmd, password);
		strcat(passwordCmd, "\r\n"); //CRLF

		//Send password to server
		write(clientSd, (char *)&passwordCmd, strlen(passwordCmd));
		//print response
		response = getServerResponse();
		cout << response;
		cout << endl;

		string error = "501";

		//If it is a valid password break out of loop
		if(!strstr(response, error.c_str())) {
			break;
		}
	}
	cin.ignore();
}

//Socket polling function after user enters correct password
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
    strcat(userCmd, "\r\n"); //CRLF

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
    loggedIn = true;
}