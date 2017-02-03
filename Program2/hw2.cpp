#include <iostream>
#include "UdpSocket.h"
#include "Timer.h"
#include <vector>
using namespace std;

#define PORT 51020       // my UDP port
#define MAX 200000        // times of message transfer
#define MAXWIN 30        // the maximum window size
#define LOOP 10          // loop in test 4 and 5
#define TIMEOUT 1500     //Timeout for stop-and-wait

// client packet sending functions
void clientUnreliable( UdpSocket &sock, const int max, int message[] );
int clientStopWait( UdpSocket &sock, const int max, int message[] );
int clientSlidingWindow( UdpSocket &sock, const int max, int message[], 
			  int windowSize );
//int clientSlowAIMD( UdpSocket &sock, const int max, int message[],
//		     int windowSize, bool rttOn );

// server packet receiving fucntions
void serverUnreliable( UdpSocket &sock, const int max, int message[] );
void serverReliable( UdpSocket &sock, const int max, int message[] );
void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], 
			 int windowSize );
//void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], 
//			 int windowSize, bool congestion );

enum myPartType { CLIENT, SERVER, ERROR } myPart;

int main( int argc, char *argv[] ) {

  int message[MSGSIZE/4]; // prepare a 1460-byte message: 1460/4 = 365 ints;
  UdpSocket sock( PORT );  // define a UDP socket

  myPart = ( argc == 1 ) ? SERVER : CLIENT;

  if ( argc != 1 && argc != 2 ) {
    cerr << "usage: " << argv[0] << " [serverIpName]" << endl;
    return -1;
  }

  if ( myPart == CLIENT ) // I am a client and thus set my server address
    if ( sock.setDestAddress( argv[1] ) == false ) {
      cerr << "cannot find the destination IP name: " << argv[1] << endl;
      return -1;
    }

  int testNumber;
  cerr << "Choose a testcase" << endl;
  cerr << "   1: unreliable test" << endl;
  cerr << "   2: stop-and-wait test" << endl;
  cerr << "   3: sliding windows" << endl;
  cerr << "--> ";
  cin >> testNumber;

  if ( myPart == CLIENT ) {

    Timer timer;           // define a timer
    int retransmits = 0;   // # retransmissions

    switch( testNumber ) {
    case 1:
      timer.start( );                                          // start timer
      clientUnreliable( sock, MAX, message );                  // actual test
      cerr << "Elasped time = ";                               // lap timer
      cout << timer.lap( ) << endl;
      break;
    case 2:
      timer.start( );                                          // start timer
      retransmits = clientStopWait( sock, MAX, message );      // actual test
      cerr << "Elasped time = ";                               // lap timer
      cout << timer.lap( ) << endl;
      cerr << "retransmits = " << retransmits << endl;
      break;
    case 3:
      for ( int windowSize = 1; windowSize <= MAXWIN; windowSize++ ) {
	timer.start( );                                        // start timer
	retransmits =
	clientSlidingWindow( sock, MAX, message, windowSize ); // actual test
	cerr << "Window size = ";                              // lap timer
	cout << windowSize << " ";
	cerr << "Elasped time = "; 
	cout << timer.lap( ) << endl;
	cerr << "retransmits = " << retransmits << endl;
      }
      break;
    default:
      cerr << "no such test case" << endl;
      break;
    }
  }
  if ( myPart == SERVER ) {
    switch( testNumber ) {
    case 1:
      serverUnreliable( sock, MAX, message );
      break;
    case 2:
      serverReliable( sock, MAX, message );
      break;
    case 3:
      for ( int windowSize = 1; windowSize <= MAXWIN; windowSize++ )
	serverEarlyRetrans( sock, MAX, message, windowSize );
      break;
    default:
      cerr << "no such test case" << endl;
      break;
    }

    // The server should make sure that the last ack has been delivered to
    // the client. Send it three time in three seconds
    cerr << "server ending..." << endl;
    for ( int i = 0; i < 10; i++ ) {
      sleep( 1 );
      int ack = MAX - 1;
      sock.ackTo( (char *)&ack, sizeof( ack ) );
    }
  }

  cerr << "finished" << endl;

  return 0;
}

// Test 1: client unreliable message send -------------------------------------
void clientUnreliable( UdpSocket &sock, const int max, int message[] ) {
  cerr << "client: unreliable test:" << endl;

  // transfer message[] max times
  for ( int i = 0; i < max; i++ ) {
    message[0] = i;                            // message[0] has a sequence #
    sock.sendTo( ( char * )message, MSGSIZE ); // udp message send
    cerr << "message = " << message[0] << endl;
  }
}

// Test1: server unreliable message receive -----------------------------------
void serverUnreliable( UdpSocket &sock, const int max, int message[] ) {
  cerr << "server unreliable test:" << endl;

  // receive message[] max times
  for ( int i = 0; i < max; i++ ) {
    sock.recvFrom( ( char * ) message, MSGSIZE );   // udp message receive
    cerr << message[0] << endl;                     // print out message
  }
}

// Test2: stop and wait client send
int clientStopWait( UdpSocket &sock, const int max, int message[] ) {
  cerr << "client stop-and-wait test" << endl;

  Timer *ackTimer = new Timer();
  int numRetransmissions = 0;
  
  for(int i = 0; i < max; i++) {
    message[0] = i;                             //message[0] has a sequence #
    sock.sendTo( ( char * )message, MSGSIZE ); // udp message send

    ackTimer->start();

    //Check for ack from server
    while(sock.pollRecvFrom() < 1) {
        //have we timed out?
        if(ackTimer->lap() >= TIMEOUT) {
          numRetransmissions++;
          //Resend message
          sock.sendTo( ( char * )message, MSGSIZE ); // udp message send
          //Restart timer
          ackTimer->start();
        }
    }

    //Read ack
    sock.recvFrom( ( char * ) message, MSGSIZE );   // udp message receive

    //cerr << "message = " << message[0] << endl;
  }
  cerr << "client done" << endl;
  //Delete ptr
  delete ackTimer;
  return numRetransmissions;
}

// Test2: server stop and wait recieve
void serverReliable( UdpSocket &sock, const int max, int message[] ) {
  cerr << "server reliable test" << endl;

  for(int seqNum = 0; seqNum < max; seqNum++) {
    //Keep recieving messages and sending acks until server
    // and client are at same sequence number 
    do {
      sock.recvFrom( ( char * ) message, MSGSIZE );   // udp message receive
      if(message[0] == seqNum) {
        //send ack
        sock.ackTo( (char *) &seqNum, sizeof(seqNum));   
      }
    //Check for wrong sequence number on message
    }while(message[0] != seqNum);
  }
}

// Test3: sliding window client
int clientSlidingWindow( UdpSocket &sock, const int max, int message[], int windowSize ) {
  Timer *ackTimer = new Timer();
  int numRetransmissions = 0; 
  int ack = -1;                          // prepare a space to receive ack
  int ackSeq = 0;                       // the ack sequence expected to receive 

  // transfer message[] max times
  for ( int sequence = 0; sequence < max || ackSeq < max; ) {  
    //send till sliding window full
    if ( ackSeq + windowSize > sequence && sequence < max ) {
      message[0] = sequence;                    
      sock.sendTo( (char *)message, MSGSIZE );  
      // check if ack arrived and if ack is the same as ackSeq, increment ackSeq
      if(sock.pollRecvFrom() > 0) {
        sock.recvFrom( (char * ) &ack, sizeof(ack));
        if(ack == ackSeq) {
          ackSeq++;
        }
      }
      // increment sequence
      sequence++;
    } else { 
      //window is full

      //check for ack until timeout
      ackTimer->start();
      while(sock.pollRecvFrom() < 1) {
        if(ackTimer->lap() >= TIMEOUT) {
          //Timeout (ack lost)
          numRetransmissions +=  sequence - ackSeq;
          sequence = ackSeq;
        }
      }
      //Ack recieved late
      sock.recvFrom( (char * ) &ack, sizeof(ack));
      if(ack >= ackSeq) {
        ackSeq = ack + 1;
      }
    }
  }
  delete ackTimer;
  return numRetransmissions;
}

// Test3: sliding window server
void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], int windowSize ) {

  //recieved[i] == true if if message[i] is recieved init to false
  vector<bool> recieved(max, false); 
  for(int seqNum = 0; seqNum < max;) {
    sock.recvFrom( (char *)message, MSGSIZE );
    if(message[0] == seqNum) {
      recieved[seqNum] = true;
    } else {
      recieved[message[0]] = true;
    }
    int maxSeq = seqNum + windowSize;
    if(maxSeq > max) {
      maxSeq = max;
    }
    //Send back acks for recieved messages in window
    for(int i = seqNum; i < maxSeq; i++) {
      if(recieved[i]) {
        sock.ackTo( (char *) &seqNum, sizeof(seqNum));
      }
    }
    //Increment seqNum as needed
    while(recieved[seqNum]) {
        seqNum++;
    }
  } 
   
}