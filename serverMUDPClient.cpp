#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#define UDPSERVERPORT 21112
#define IPADDR "127.0.0.1"
#define DOMAIN PF_INET // TODO change domain to AF_INET for Unix

using namespace std;

int main()
{
  // create a hint structure for the server
  sockaddr_in datagram_server_hint;
  datagram_server_hint.sin_family = DOMAIN;
  datagram_server_hint.sin_port = htons(UDPSERVERPORT);
  inet_pton(DOMAIN, IPADDR, &datagram_server_hint.sin_addr);

  // create datagram client socket
  int datagram_sock = socket(DOMAIN, SOCK_DGRAM, 0);

  // Read user input
  string userInput;
  do
  {
    cout << "> ";
    getline(cin, userInput);

    // send data
    int sendData = sendto(datagram_sock,
                          userInput.c_str(),
                          userInput.size() + 1,
                          0,
                          (sockaddr *)&datagram_server_hint,
                          sizeof(datagram_server_hint));

    if (sendData == -1)
    {

      cerr << "UDP client serverM could not send to UDP server "
           << IPADDR
           << " on port "
           << UDPSERVERPORT
           << " Please try again." << endl;
      continue;
    }
  } while (true);

  // Close Socket
  close(datagram_sock);
  return 0;
}