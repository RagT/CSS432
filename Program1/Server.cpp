#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <string.h>       // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

const int BUFSIZE = 1500;
int repetitions = -1;
int port = -1;
int serverSd; //Socket descriptor
int newSd;	//connection request specific socket descriptor

void handlerFunction(int arg) {
	char dataBuff[BUFSIZE];

	struct timeval start;
	struct timeval end;
	long timeElapsed;

	//Start the timer
	gettimeofday(&start, NULL);

	int count = 0;
	//Get number of reads
	for(int i = 0; i < repetitions; i++) {
		int nRead = 0;
		while(nRead < BUFSIZE) {
			nRead += read(newSd, dataBuff, BUFSIZE - nRead);
			count++;
		}
	}

	//End the timer
	gettimeofday(&end, NULL);

	//Calculate time elapsed
	timeElapsed = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	cout << "data-receiving time =  " << timeElapsed << " usec" << endl;

	//Send number of read() calls back to the client as an acknowledge
	write(newSd, &count, sizeof(count));

	//Stop server process
	close(newSd);
	close(serverSd);
	exit(0);
}

int main(int argc, char * argv[]) {
	//Check validity of command line arguments

	if(argc != 3) {
		cerr << "Invalid number of arguments." << endl;
		cerr << endl;
		cerr << "There were two arguments expected:" << endl;
		cerr << "port# 			The ip port for server to use." << endl;
		cerr << "repetitions 	The number of times to send a set of data buffers." << endl;
		return -1;
	}

	port = atoi(argv[1]);
	repetitions = atoi(argv[2]);

	if(port < 1024 || port > 65536) {
		cerr << "Port outside of allowable port range of 1024 to 65536." << endl;
		return -1;
	} 

	if(repetitions < 0){
		cerr << "Number of repetitions must be positive." << endl;
		return -1;
	}

	sockaddr_in acceptSockAddr;
	bzero((char*)acceptSockAddr, sizeof(acceptSockAddr));
	acceptSockAddr.sin_family = AF_INET;
	acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	acceptSockAddr.sin_port = htons(port);

	//Open stream oriented socket with Internet address family
	serverSd = socket(AF_INET, SOCK_STREAM, 0); 
	if(serverSd < 0) {
		cerr << "Cannot open a server TCP socket." << endl;
		return -1;
	}

	//prompt OS to release server port as soon as process terminates
	const int on = 1;
	setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int));

	//Bind socket to local address
	if(bind(serverSd, (sockaddr *)&acceptSockAddr, sizeof(&acceptSockAddr)) < 0) {
		cerr << "Unable to bind socket to local address" << endl;
		close(serverSd);
		return -1;
	}

	//Instruct OS to listen to up to fve connection requests from clients
	listen(serverSd, 5);

	//Recieve a request from a client by calling accept that will return a new socket
	//specific to this connection request.
	sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int newSd = accept( serverSd, (sockaddr*)&newSockAddr, &newSockAddrSize);

    signal(SIGIO, handlerFunction);

    //Change socket to asynchronous connection
    fcntl(newSd, F_SETOWN, getpid());
    fcntl(newSd, F_SETFL, FASYNC);

    while(true) {
    	sleep(1000);
    }
    return 0;
}


