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
#include <errno.h>
using namespace std;

const int BUFSIZE = 1500;

int main(int argc, char * argv[]) {
	//Check validity of command line arguments

	if(argc != 7) {
		cerr << "Incorrect number of arguments." << endl;
		cerr << endl;
		cerr << "There were 6 arguments expected." << endl;
		cerr << "port 			The port number to connect to" << endl;
		cerr << "repetitions 	The number of times to send a set of data buffers" << endl;
		cerr << "nbufs 			The number of data buffers" << endl;
		cerr << "bufsize   		the size of each data buffer (in bytes)" << endl;
		cerr << "serverIp 		A server Ip name" << endl;
		cerr << "types 			type of transfer scenario: 1,2,or 3" << endl;
		cerr << endl;
        return -1;
	}

	int port = atoi(argv[1]);
	int repetitions = atoi(argv[2]);
	int nbufs = atoi(argv[3]);
	int bufsize = atoi(argv[4]);
	const char * serverIp = argv[5] + '\0';
	int type = atoi(argv[6]);

	if(port < 1024 || port > 65536) {
		cerr << "Port not in allowable port range between 1024 and 65536" << endl;
		return -1;
	} 

	if(repetitions < 0){
		cerr << "Must have positive number of repetitions" << endl;
		return -1;
	}

	if(nbufs * bufsize != BUFSIZE) {
		cerr << "nbufs * bufsize must equal 1500" << endl;
		return -1;
	}

	if(type < 1 || type > 3) {
		cerr << "Type must be 1,2, or 3" << endl;
		return -1;
	}

	struct hostent* host = gethostbyname(serverIp);
	if(host == NULL) {
		cerr << "Invalid hostname" << endl;
		return -1;
	}

	sockaddr_in sendSockAddr;
	bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
	sendSockAddr.sin_family = AF_INET; // Address Family Internet
    sendSockAddr.sin_addr.s_addr =
    	inet_addr( inet_ntoa( *(struct in_addr*) (*host->h_addr_list) ) );
    sendSockAddr.sin_port = htons(port);

    //Open a stream oriented socket with Internet address family
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSd < 0) {
    	cerr << "Could not open a socket" << endl;
    	close(clientSd);
    	return -1;
    }

    //Connect socket to the server
    if(connect(clientSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) < 0) {
    	cerr << "Failed to connect to server" << endl;
        cerr << strerror(errno) << endl;
    	close(clientSd);
    	return -1;
    }

    cout << "Connected!" << endl;
    //Allocate databuffer
    char databuffer[nbufs][bufsize];

    //Start a timer
    struct timeval start;
    struct timeval end;
    struct timeval lap;
    long lapTime;
    long roundTripTime;

    //Start time
    gettimeofday(&start, NULL);
    cout << "entering loop" << endl;
    for(int i = 0; i < repetitions; i++) {
        //multiple writes: invokes the write( ) system call for each data buffer, 
        //thus resulting in calling as many write( )s as the number of data buffers, 
        //(i.e., nbufs).
    	if(type == 1) {
            for ( int j = 0; j < nbufs; j++ ) {
                write(clientSd, databuffer[j], bufsize);
            }
	    cout << "write completed" << endl;
        }
        //writev: allocates an array of iovec data structures, each having its 
        //*iov_base field point to a different data buffer as well as storing the buffer
        // size in its iov_len field; and thereafter calls writev( ) 
        //to send all data buffers at once.
        if(type == 2) {
            struct iovec vector[nbufs];
            for(int j = 0; j < nbufs; j++) {
                vector[j].iov_base = databuffer[j];
                vector[j].iov_len = bufsize;
            }
            writev(clientSd, vector, nbufs);
            cout << "write completed" << endl;
        }
        //single write: allocates an nbufs-sized array of data buffers, and thereafter calls
        // write( ) to send this array, (i.e., all data buffers) at once.
        if(type == 3) {
            write(clientSd, databuffer, nbufs * bufsize);
            cout << "write completed" << endl;
        }
    }

    //write lap time
    gettimeofday(&lap, NULL);

    //Recieve acknowledge from server on how many times read was called
    int readCount;
    read(clientSd, &readCount, sizeof(readCount));

    //Record round trip end time (RTT)
    gettimeofday(&end, NULL);

    //Calculate lap and round trip time
    lapTime = (lap.tv_sec - start.tv_sec) * 1000000 + (lap.tv_usec - start.tv_usec);
	roundTripTime = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

    //Print out stats to terminal
    cout << "Test " << type << ": data-sending time = " << lapTime << " usec, ";
    cout << "round-trip time = " << roundTripTime << " usec, " << "#reads = " << readCount << endl;

    //close the socket
    close(clientSd);
    return 0;
}
