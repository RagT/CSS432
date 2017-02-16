#include "Socket.h"
#include <stdlib.h>

using namespace std;

void server( ); 
void client( char ipName[] );
void usage( char progName[] );

Socket *sock;

int main( int argc, char* argv[] ) {
  if ( argc > 1 ) {
    sock = new Socket( atoi( argv[1] ) );
    if ( argc == 2 )
      server( );
    else if ( argc == 3 )
      client( argv[2] );
  }
  else {
    usage( argv[0] );
    return -1;
  }
  return 0;
}

void server( ) {

  // Get a server sd
  int serverSd = sock->getServerSocket( );

  // Exchange data
  char message[1500];
  read( serverSd, message, 1500 );
  write( serverSd, message, 1 );

  // Close socket but not send FIN.
  close( serverSd );
}

void client( char ipName[] ) {

  // Get a client sd
  int clientSd = sock->getClientSocket( ipName );

  // Exchange data
  char message[1500];
  write( clientSd, message, 1500 );
  read( clientSd, message, 1 );

  // Close socket to send FIN.
  close( clientSd );
}

void usage( char progName[] ) {
  cerr << "usage:" << endl;
  cerr << "server invocation: " << progName << " ipPort" << endl;
  cerr << "client invocation: " << progName << " ipPort ipName" << endl;
}
