// Project:      CSS432 Option Class
// Professor:    Munehiro Fukuda
// Organization: CSS, University of Washington, Bothell
// Date:         September 14, 2004

#ifndef _OPTION_H_
#define _OPTION_H_

#define BUFLEN 4096
#define SOCKBUFSIZE 16384
#define PORT 5001
#define NBUF 2048

#include <iostream>

using namespace std;

extern "C"
{
#include <stdio.h>
}

class Option {
 public:
  Option( ) :
    transmit( false ), receive( false ), tcp_nodelay( false ), help( false ), 
    buflen( BUFLEN ), sockbufsize( SOCKBUFSIZE ), port( PORT ), nbuf( NBUF ),
    host( NULL )
    { };
  bool transmit;
  bool receive;
  bool tcp_nodelay;
  bool help;
  int buflen;
  int sockbufsize;
  int port;
  int nbuf;
  char *host;
};

#endif
