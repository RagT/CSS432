/*
Raghu Tirumala

Spoofcheck.cpp

A program that enables a server to check the integrity of a client connection 
by looking for IP address spoofing.
*/

#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <string.h>       // bzero
#include <netinet/tcp.h>  // TCP_NODELAY
#include <iostream>      // cerr
using namespace std;

const int MAX_CONNECTIONS = 5;

int main(int argc, char const *argv[])
{
	if(argc != 2) {
		cerr << "Invalid number of arguments" << endl;
		cerr << "Must pass portnumber to bind to" << endl;
		return -1;
	}

	int portnumber = atoi(argv[1]);
	if(portnumber < 1024 || portnumber > 65536) {
		cerr << "Portnumber must be between 1024 and 65536." << endl;
		return -1;
	}

	//Step 1: Instantiate a TCP Socket

	//Build socket address
	sockaddr_in server;
	bzero((char *) server, sizeof(server));
	server.sin_family =AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(portnumber);

	//Open socket
	int serverSd = socket(AF_INET, SOCK_STREAM, 0);
	const int on = 1;
	setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));

	//Bind socket
	if(bind(serverSd, (sockaddr *)&server, sizeof(server)) < 0){
		cerr << "Unable to bind socket." << endl;
		return -1;
	}

	//Listen for connections
	listen(serverSd, MAX_CONNECTIONS);

	while(true) {
		//Step 2: Accept a new connection from a client through accept().
		sockaddr_in client;
        int newSd = accept(serverSd, (sockaddr *) &client, sizeof(client));

        //Step 3: Spawn a child process through fork().
        //The parent closes this connection and goes back to the top of the loop, 
        //whereas the child continues checking the integrity of this connection.
        
        //The child process
        if(fork() == 0) {
        	close(serverSd);

        	//Step 4: Retrieve the client's IP address 
        	//and port of this connection through getpeername().
        	getpeername(newSd, (sockaddr *)&client, sizeof(client));

        	char * clientIp = inet_ntoa(client.sin_addr);
        	int clientPort = ntohs(client.sin_port);
        	cout << "client addr = " << clientIp << " port = " << clientPort << endl;

        	//Step 5: Retrieve the client's hostent data structure through 
        	//gethostbyaddr().
        	unsigned int addr = inet_addr(clientIp);

        	struct hostent* hostPtr = gethostbyaddr((const void *) addr, 
        							sizeof(unsigned int), AF_INET);
        	//Unable to gethostbyaddr
        	if( hostPtr == NULL) {
        		cout << "gethostbyaddr error for client(" << clientIp << "): 1" << endl;
        		cout << "a spoofing client" << endl;
        		cout << endl;
        	} else {
        		//Step 6: Retrieve the client's official name, aliases, 
        		//and registered IP addresses from the hostent
        		cout << "official hostname: " << hostPtr->h_name << endl;

        		//Get list of aliases
        		char **aliases = hostPtr->h_aliases;
        		int numAliases = 0;
        		while(*aliases != NULL) {
        			cout << "alias: " << *aliases << endl;
        			numAliases++;
        		}
        		if(numAliases == 0){
        			cout << "alias: none" << endl;
        		}

        		short h_addrtype = hostPtr->h_addrtype;

        		if(h_addrtype == AF_INET) {
        			char **addressList = hostPtr->h_addr_list;
        			char * registeredIp = NULL;
        			int listSize; //size of address list
        			bool isHonest = false;

        			//Step 7: Decide whether this client is a honest or a spoofing client 
        			//by matching its IP address retrieved from getpeername() and the list 
        			//of addresses retrieved via gethostbyaddr().

        			for(listSize = 0; addressList[listSize] != NULL; listSize++) {
        				registeredIp = inet_ntoa((in_addr *) addressList[i]);
        				cout << "ip address: " << registeredIp << " ... hit!" << endl;
        				if(registeredIp == clientIp) {
        					isHonest = true;
        				}
        			}
        			
        			if(isHonest) {
        				cout << "an honest client" << endl;
        			} else {
        				cout << "a spoofing client" << endl;
        			}
        			cout << endl;

        		} else {
        			cerr << "Unknown address type." << endl;
        		}
        	}
        	//Step 8: Terminate child process
        	close(newSd);
        	exit(0);
        } else {
        	//Parent process closes connection
        	close(newSd);
        }
	}
	close(serverSd);
	return 0;
}