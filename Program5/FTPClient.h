#include "Socket.h"
#include <string.h>
#include <iostream>
#include <sys/poll.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
using namespace std;

#define BUF_SIZE = 8192;

class FTPClient {
public:

private:
	int clientSd;
	int passiveSd;
	Socket * clientSocket;
	Socket * serverSocket;
	
}