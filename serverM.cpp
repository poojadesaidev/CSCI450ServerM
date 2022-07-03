#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/wait.h>

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#define TCPPORT 25112
#define UDPPORT 24112
#define DOMAIN PF_INET // TODO change domain to AF_INET for Unix
#define MAXBUFLEN 4096
#define IPADDR "127.0.0.1"

using namespace std;

int main()
{

  // Stream Sock Server (TCP socket server)

  // Create Welcoming Socket
  int stream_welcoming_sock; // socket id of welcoming socket
  // Domain = AF_INET  Type = SOCK_STREAM Protocol = 0
  // We let the transport layer decide the protocol based on 'Type'

  if ((stream_welcoming_sock = socket(DOMAIN, SOCK_STREAM, 0)) == -1)
  {
    cerr << "Stream Welcoming socket could not be created for ServerM";
    return -1;
  }

  // Bind the socket to the IP/Port
  sockaddr_in stream_hint; // address (IPV4) for welcoming socket
  stream_hint.sin_family = DOMAIN;
  stream_hint.sin_port = htons(TCPPORT);            // htons to do host to network translation for port#
  inet_pton(DOMAIN, IPADDR, &stream_hint.sin_addr); // inet_pton to convert a number in our IP to array of integers

  //if ((bind(stream_welcoming_sock, DOMAIN, &stream_hint, sizeof(stream_hint))) == -1)
  if ((::bind(stream_welcoming_sock, (const sockaddr *)&stream_hint, sizeof(stream_hint))) == -1)
  //if ((bind(stream_welcoming_sock, (sockaddr *)&stream_hint, sizeof(stream_hint))) == -1)
  {
    cerr << "Stream Socket IP/Port binding could not be done for ServerM";
    return -2;
  }

  // Listen and display boot message
  if (listen(stream_welcoming_sock, SOMAXCONN) == -1)
  {
    cerr << "Stream socket could not listen for ServerM";
    return -3;
  }

  // Stream socket is listening successfully
  cout << "The main server is up and running." << endl;

  // client accept loop
  while (1)
  {
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST]; // buffer to put host name - size 1025
    char svc[NI_MAXSERV];  // buffer to put service name - size 32

    // Accept or create child socket
    // pull in a request from incoming request queue and create a child socket to process it
    int childSocket = accept(stream_welcoming_sock, (sockaddr *)&client, &clientSize);

    if (childSocket == -1)
    {
      cerr << "Stream socket could not accept client for ServerM";
      continue;
      //return -4;
    }

    if (!fork())
    { // this is a child process
      // Child socket doesnot need the welcoming socket or listener
      close(stream_welcoming_sock);

      // Clean up host and svc before populating it
      memset(host, 0, NI_MAXHOST);
      memset(svc, 0, NI_MAXSERV);

      // get name of clients computer
      int result = getnameinfo((sockaddr *)&client,
                               sizeof(client),
                               host,
                               NI_MAXHOST,
                               svc,
                               NI_MAXSERV,
                               0);

      // GETNAMEINFO WAS SUCCESSFUL
      if (result)
      {
        cout << host << " connected on " << svc << endl;
      }
      else
      {
        // read clients addr and put it in host array
        // ideally getnameinfo should have done this for us as well as populating the svc
        // if getnameinfo fails, we have to manully do this ourselves
        inet_ntop(DOMAIN, &client.sin_addr, host, NI_MAXHOST);              // convert numeric array to string
        cout << host << " connected on " << ntohs(client.sin_port) << endl; // ntohs is network to host short
      }

      // TCP connection is opened, so now exchange messages
      char buf[MAXBUFLEN];
      while (true)
      {
        // Clear the buffer
        memset(buf, 0, MAXBUFLEN);

        // Recieve message
        int bytesRecv = recv(childSocket, buf, MAXBUFLEN - 1, 0);
        if (bytesRecv == -1)
        {
          cerr << "Stream child socket could not recieve msg from client on ServerM" << endl;
          break;
        }

        if (bytesRecv == 0)
        {
          cout << "The client disconnected on ServerM" << endl;
          break;
        }

        // Display message that was recieved
        cout << "Received " << string(buf, 0, bytesRecv) << endl;

        // Send message
        if (send(childSocket, buf, bytesRecv + 1, 0) == -1)
        {
          cerr << "Error sending message from ServerM to " << host << " who was requesting for service " << svc << endl;
          break;
        }
      }
      // Close child socket
      close(childSocket);
      exit(0);
    }

    // Close child socket as parent does not need it
    close(childSocket);
  }

  // Close welcoming socket
  close(stream_welcoming_sock);
  return 0;
}
