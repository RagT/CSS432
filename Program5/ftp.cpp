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
int pasvSd = -1;
int clientFd = -1;
const int CTRL_PORT = 21; //Port for FTP communication
bool loggedIn = false;
const int BUF_SIZE = 8192;
char buff[BUF_SIZE];
struct hostent *host;
//variables to store server response data when going into passive mode
int A1, A2, A3, A4, a1, a2; 
int pid; //for fork()

void getServerResponse();
void getPassword();
void logIn();
void checkLogIn();
void close();
bool pasv();
void setTypeToI();
void get(string filename);
void put();

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
		if(command.empty()) {
			continue;
		}
		//vector to store each part of command 
		vector<string> splitCommand = split(command);
		string commandType = splitCommand[0];

		if(commandType == "open") {
			if(!loggedIn) {
				logIn();
			} else {
				cout << "You are already logged in." << endl;
			}
		} else if(commandType == "ls") {
			checkLogIn();
			if(!loggedIn) {
				continue;
			}
			if(!pasv()) {
				continue;
			}
			string directoryList;
			if((pid = fork()) < 0) {
				cerr << "Fork failed." << endl;
				continue;
			} else if (pid > 0) {
				//parent process
				char listCmd[BUF_SIZE];
				strcpy(listCmd, "LIST");
				strcat(listCmd, "\r\n");
				write(clientSd, (char *) &listCmd, strlen(listCmd));
				wait(NULL); //wait for child process
			} else {
				//child process
				
				//clear buffer before writing to it
				bzero(buff, sizeof(buff));
				//list of directories and files sent through passive connection
				while(read(pasvSd, (char *)&buff, sizeof(buff)) > 0) {
					directoryList.append(buff);
				}
				getServerResponse();
				cout << buff;
				getServerResponse();
				cout << buff;
				cout << directoryList << endl;
				exit(0);
			}
			close(pasvSd);

		}  else if(commandType == "cd") {
			checkLogIn();
			if(splitCommand.size() != 2) {
				cout << "Usage: cd subdir" << endl;
				continue;
			}
			if(!loggedIn) {
				continue;
			}
			char cdCommand[BUF_SIZE];
			strcpy(cdCommand, "CWD ");
			strcat(cdCommand, splitCommand[1].c_str());
			strcat(cdCommand, "\r\n");

			//send command to server
			write(clientSd, (char *)&cdCommand, strlen(cdCommand));
			getServerResponse();
			cout << buff;
			continue;

		} else if( commandType == "get") {
			checkLogIn();
			if(!loggedIn) {
				continue;
			}
			if(splitCommand.size() != 2) {
				cout << "Usage: requires 1 argument, the file to get." << endl;
			}
			get(splitCommand[1]);
		} else if(commandType == "put") {
			checkLogIn();
			if(!loggedIn) {
				continue;
			}
			if(splitCommand.size() != 1) {
				cout << "Usage: just type put" << endl;
				continue;
			}
			put();
		} else if(commandType == "close") {
			//Close connection to server but keep ftp client running
			//Check if connection is open first
			checkLogIn(); 
			if(loggedIn) {
				close();
				loggedIn = false;
			}
		} else if(commandType == "quit") {
			//log out of server and quit ftp client
			if(loggedIn) {
				close(); //close connection with ftp server
			}
			break;   //Exit program
		} else if(commandType == "help"){
			cout << "FTP Client commands:" << endl;
			cout << "open:  Opens connection to ftp server" << endl;
			cout << "cd subdir:   Change directory to subdirectory" << endl;
			cout << "ls:	 List all files and subdirectories in current directory" << endl;
			cout << "get filename:   Get specified file from server to local system." << endl;
			cout << "put:   Requests filename of local file and new filename. Places copy of local file with the new filename." << endl;
			cout << "close:   Closes connection to ftp server." << endl;
			cout << "quit :	 Closes connection to ftp server and quits this client program" << endl;
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

//Reads response from server into char[] buffer
void getServerResponse() {
	//Clear the buffer
	bzero(buff, sizeof(buff));
	//Read data from server into buffer
	read(clientSd, buff, sizeof(buff));
	string error = "421";
	if(strstr(buff, error.c_str())) {
		cerr << "There was a problem with the server." << endl;
		close();
		exit(0);
	}
}

//Prompts user for their password until correct password is recieved.
void getPassword() {
	while(true) {
		cout << "Password: ";
		char password[BUF_SIZE];
		cin >> password;
		char passwordCmd[BUF_SIZE];
		strcpy(passwordCmd, "PASS ");
		strcat(passwordCmd, password);
		strcat(passwordCmd, "\r\n"); //CRLF

		//Send password to server
		write(clientSd, (char *)&passwordCmd, strlen(passwordCmd));
		//print response
		getServerResponse();
		cout << buff;

		string error = "501";

		//If it is a valid password break out of loop
		if(!strstr(buff, error.c_str())) {
			break;
		}
	}
	//Clear buffer of newline chars so getline() can work properly
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
	
	host = gethostbyname(ftpServerName);
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
    getServerResponse();
    cout << buff;

    //Log in user to server

    //Request username
    cout << "Name(" << ftpServerName << ":" << getenv("USER") << "): ";
    char name[BUF_SIZE];
    cin >> name;
    char userCmd[BUF_SIZE];
    strcpy(userCmd, "USER ");
    strcat(userCmd, name);
    strcat(userCmd, "\r\n"); //CRLF

    //Send username to server and recieve the acknowledgement
    write(clientSd, (char*)&userCmd, strlen(userCmd));
    getServerResponse();
    cout << buff;

    //Enter password
    getPassword();

    //Poll the server
    while(pollSocket() == 1) {
        getServerResponse();
    	cout << buff;
    }
    if(pollSocket() == -1) {
    	cout << "Can't poll socket" << endl;
    }
    loggedIn = true;
}

void close() {
	char closeCmd[BUF_SIZE];
	strcpy(closeCmd, "QUIT");
	strcat(closeCmd, "\r\n");
	write(clientSd, (char*)&closeCmd, strlen(closeCmd));
	getServerResponse();
	cout << buff;
	shutdown(clientSd, SHUT_WR); //close socket
}

//Passive mode connection with FTP server returns false if there is an error
bool pasv() {
	char passiveCmd[BUF_SIZE];
	strcpy(passiveCmd, "PASV");
	strcat(passiveCmd, "\r\n");
	write(clientSd, (char*)&passiveCmd, strlen(passiveCmd));
	getServerResponse();
	cout << buff;

	sscanf(buff, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", 
			&A1, &A2, &A3, &A4, &a1, &a2);

	int serverPortNum = (a1 * 256) + a2;
	struct sockaddr_in serverAddress;
	bzero((char*) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPortNum);
    serverAddress.sin_addr.s_addr = 
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));

	pasvSd = socket(AF_INET, SOCK_STREAM, 0);
	if(pasvSd < 0) {
		cerr << "Error creating passive socket" << endl;
		return false;
	}

	//passive connection to ftp server
	if(connect(pasvSd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
		cerr << "Can't establish passive connection" << endl;
		return false;
	}
	return true;
}

//Sets data sending type to binary for file retrieval
void setTypeToI() {
	char typeCmd[BUF_SIZE];
	strcpy(typeCmd, "TYPE I");
	strcat(typeCmd, "\r\n");
	write(clientSd, (char*) &typeCmd, strlen(typeCmd));
	getServerResponse();
	cout << buff;
}

//Get command implementation
void get(string filename) {
	//Set type to 'I' (IMAGE aka binary)
	setTypeToI();
	//establish pasv connection
	if(!pasv()) {
		return;
	}
	int file;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	if((pid = fork()) < 0) {
		cerr << "Get: fork failed" << endl;
		return;
	} else if(pid > 0) {
		//parent process
		char retrieveCmd[BUF_SIZE];
		strcpy(retrieveCmd, "RETR ");
		strcat(retrieveCmd, filename.c_str());
		strcat(retrieveCmd, "\r\n");
		write(clientSd, (char*) &retrieveCmd, strlen(retrieveCmd));
		wait(NULL); //wait for child
	} else {
		//child process
		file = open(filename.c_str(), O_WRONLY | O_CREAT, mode);
			bzero(buff, sizeof(buff));
		while(true) {
			int n = read(pasvSd, buff, sizeof(buff));
			if(n == 0) {
				break;
			}
			write(file, buff, n);
		}
		close(file);
		exit(0);
	}
	close(pasvSd);
	getServerResponse();
	cout << buff;
	getServerResponse();
	cout << buff;
}

void put() {
	//get filename and newFilename from user
	cout << "(local file) ";
	string filename;
	getline(cin, filename);
	cout << "(remote file) ";
	string newFilename;
	getline(cin, newFilename);  

	//Set type to 'I' (IMAGE aka binary)
	setTypeToI();
	//establish pasv connection
	if(!pasv()) {
		return;
	}

	if((pid = fork()) < 0) {
		cerr << "put: fork failed" << endl;
		return;
	} else if(pid > 0) {
		//parent process
		char putCmd[BUF_SIZE];
		strcpy(putCmd, "STOR ");
		strcat(putCmd, newFilename.c_str());
		strcat(putCmd, "\r\n");
		write(clientSd, (char *) &putCmd, strlen(putCmd));
		wait(NULL); //wait for child
	} else {
		//child process
		int file = open(filename.c_str(), O_RDONLY);
		while(true) {
			int n = read(file, buff, sizeof(buff));
			if(n == 0){
				break;
			}
			write(pasvSd, buff, n);
		}
		close(file);
		exit(0);	
	}
	close(pasvSd);
	getServerResponse();
	cout << buff;
	getServerResponse();
	cout << buff;
}