#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

int main()
{
  int welcoming_sock;
  // Create Welcoming Socket
  // Domain = PF_INET  Type = SOCK_STREAM Protocol = 0
  // We let the transport layer decide the protocol based on 'Type'
  // TODO change domain to AF_INET for Unix
  if ((welcoming_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
  {
    cerr << "Welcoming socket could not be created for ServerA";
    return -1;
  }

  //Bind the socket

  //Listen and display boot message

  //Accept or create child socket

  //Recieve message

  //Send message

  //Close child socket
}
