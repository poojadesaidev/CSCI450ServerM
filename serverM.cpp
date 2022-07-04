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
#include <sstream>
#include <vector>
#include <cstring>
#include <ctype.h>
#include <cstdlib>

#define TCPPORT 25112
#define UDPPORT 24112
#define DOMAIN PF_INET // TODO change domain to AF_INET for Unix
#define MAXBUFLEN 4096
#define IPADDR "127.0.0.1"
#define UDPPORTSERVERA 21112
#define UDPPORTSERVERB 22112
#define UDPPORTSERVERC 23112

using namespace std;

string encode(string originalString)
{

  int n = originalString.length();

  // declaring character array
  char original_char_array[n + 1];
  char encoded_char_array[n];

  // copying the contents of the
  // string to char array
  strcpy(original_char_array, originalString.c_str());

  for (int i = 0; i < n + 1; i++)
  {
    char c = original_char_array[i];
    if (isdigit(c))
    {
      c = c + 3;
      if (c > 57)
      {
        c = c - 10;
      }
    }
    else if (isalpha(c))
    {
      c = c + 3;

      if ((c > 90 && c < 97) || c > 122)
      {
        c = c - 26;
      }
    }
    encoded_char_array[i] = c;
  }

  string s = encoded_char_array;
  return s;
  // cout << s << s.length() << endl;
}

string decode(string encodedString)
{

  int n = encodedString.length();

  // declaring character array
  char original_char_array[n + 1];
  char encoded_char_array[n + 1];

  // copying the contents of the
  // string to char array
  strcpy(encoded_char_array, encodedString.c_str());

  for (int i = 0; i < n + 1; i++)
  {
    char c = encoded_char_array[i];
    //cout << encoded_char_array[i] << endl;
    if (isdigit(c))
    {
      c = c - 3;
      if (c < 48)
      {
        c = c + 10;
      }
    }
    else if (isalpha(c))
    {
      c = c - 3;

      if ((c < 97 && c > 90) || c < 65)
      {
        c = c + 26;
      }
    }
    original_char_array[i] = c;
  }

  string s = original_char_array;

  return s;
  // cout << s << s.length() << endl;
}

int createBindListenStrmSrvrWlcmngSocket()
{

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
    return -1;
  }

  // Listen and display boot message
  if (listen(stream_welcoming_sock, SOMAXCONN) == -1)
  {
    cerr << "Stream socket could not listen for ServerM";
    return -1;
  }
  return stream_welcoming_sock;
}

int createUDPSocket()
{
  // create datagram client socket
  return socket(DOMAIN, SOCK_DGRAM, 0);
}

sockaddr_in createUDPServerAddrHint(int serverNumber)
{
  // create a hint structure for the server
  sockaddr_in datagram_server_hint;
  memset(&datagram_server_hint, 0, sizeof(datagram_server_hint));
  datagram_server_hint.sin_family = DOMAIN;
  inet_pton(DOMAIN, IPADDR, &datagram_server_hint.sin_addr); // all three servers are on localhost
  switch (serverNumber)
  {
  case 1:
    datagram_server_hint.sin_port = htons(UDPPORTSERVERA);
    return datagram_server_hint;
  case 2:
    datagram_server_hint.sin_port = htons(UDPPORTSERVERB);
    return datagram_server_hint;
  case 3:
    datagram_server_hint.sin_port = htons(UDPPORTSERVERC);
    return datagram_server_hint;
  }
  return datagram_server_hint;
}

sockaddr_in createUDPClientAddrHint()
{
  // create a hint structure for the server
  sockaddr_in datagram_client_hint;
  memset(&datagram_client_hint, 0, sizeof(datagram_client_hint));
  datagram_client_hint.sin_family = DOMAIN;
  inet_pton(DOMAIN, IPADDR, &datagram_client_hint.sin_addr); // IP for UDP client is local host
  datagram_client_hint.sin_port = htons(UDPPORT);

  return datagram_client_hint;
}

string sendRequestToDatagramServer(int datagram_client_sock,
                                   sockaddr_in datagram_server_hint,
                                   sockaddr_in datagram_client_hint,
                                   string req, int serverName)
{
  string serverTrsName;
  switch (serverName)
  {
  case 1:
    serverTrsName = "A";
    break;
  case 2:
    serverTrsName = "B";
    break;
  case 3:
    serverTrsName = "C";
    break;
  default:
    serverTrsName = "A";
    break;
  }
  socklen_t datagram_server_hint_len = sizeof(datagram_server_hint);

  // send data
  int sendData = sendto(datagram_client_sock,
                        req.c_str(),
                        req.size() + 1,
                        0,
                        (sockaddr *)&datagram_server_hint, datagram_server_hint_len);

  if (sendData == -1)
  {
    cerr << "UDP client serverM could not send to UDP server "
         << IPADDR
         << " on port "
         << ntohs(datagram_server_hint.sin_port)
         << " Please try again." << endl;
    return "invalid";
  }

  cout << "The main server sent a request to server " << serverTrsName << endl;

  char buf[MAXBUFLEN];
  // Clear the buffer and client
  memset(buf, 0, MAXBUFLEN);
  // Recieve message
  int bytesRecv = recvfrom(datagram_client_sock, buf, MAXBUFLEN - 1, 0, (sockaddr *)&datagram_server_hint, &datagram_server_hint_len);
  if (bytesRecv == -1)
  {
    cerr << "UDP client serverM could not recieve msg from UDP server "
         << IPADDR
         << " on port "
         << ntohs(datagram_server_hint.sin_port)
         << " Please try again." << endl;
    return "invalid";
  }

  if (bytesRecv == 0)
  {
    cerr << "UDP client serverM recieved empty msg from UDP server "
         << IPADDR
         << " on port "
         << ntohs(datagram_server_hint.sin_port)
         << " Please try again." << endl;
    return "invalid";
  }

  cout << "The main server received transactions from Server " << serverTrsName << " using UDP over port " << ntohs(datagram_client_hint.sin_port) << "." << endl;

  // return response that was recieved
  return string(buf, 0, bytesRecv);
}

string logTransInFileQueryServer(int serverName, int datagram_client_sock, sockaddr_in datagram_client_hint,
                                 string transactionNum, string sender, string rec, string amt)
{

  // create addressHint for server
  sockaddr_in datagram_server_hint = createUDPServerAddrHint(serverName);

  string req = "log " + transactionNum + " " + encode(sender) + " " + encode(rec) + " " + encode(amt);

  // Send request to backend Server and recieve response
  return sendRequestToDatagramServer(datagram_client_sock, datagram_server_hint, datagram_client_hint, req, serverName);
}

string checkMaxTransNumQueryServer(int serverName, int datagram_client_sock, sockaddr_in datagram_client_hint)
{

  // create addressHint for server
  sockaddr_in datagram_server_hint = createUDPServerAddrHint(serverName);

  string req = "serialnum";

  // Send request to backend Server and recieve response
  return sendRequestToDatagramServer(datagram_client_sock, datagram_server_hint, datagram_client_hint, req, serverName);
}

string checkWalletQueryServer(int serverName, string name, int datagram_client_sock, sockaddr_in datagram_client_hint)
{

  // create addressHint for server
  sockaddr_in datagram_server_hint = createUDPServerAddrHint(serverName);

  string req = "check " + encode(name);

  // Send request to backend Server and recieve response
  return sendRequestToDatagramServer(datagram_client_sock, datagram_server_hint, datagram_client_hint, req, serverName);
}

string checkWallet(string name, bool createNewSocket, int datagram_client_sock, sockaddr_in datagram_client_hint)
{
  if (createNewSocket == true)
  {
    // create Datagram Client and bind to port

    // create UDP socket
    datagram_client_sock = createUDPSocket();

    // create addressHint for serverM for bind operation
    datagram_client_hint = createUDPClientAddrHint();

    // bind UDP client to a static port
    if (::bind(datagram_client_sock, (struct sockaddr *)&datagram_client_hint, sizeof(datagram_client_hint)) == -1)
    {
      cerr << "UDP client serverM could bind to port "
           << UDPPORT
           << endl;
      // Close Socket
      close(datagram_client_sock);
      return "invalid";
    }
  }

  // Send request to Server A and recieve response
  string datagramServerResponse = checkWalletQueryServer(1, name, datagram_client_sock, datagram_client_hint);

  // process UDP response
  if (datagramServerResponse.empty() ||
      datagramServerResponse.compare("empty") == 0 ||
      datagramServerResponse.compare("invalid") == 0)
  {
    datagramServerResponse = "Unable to proceed with the transaction as \"" + name + "\" is not part of the network.";
  }
  else
  {
    int d;
    try
    {
      d = stoi(datagramServerResponse);
      int bal = 1000 + d;
      datagramServerResponse = "The current balance of \"" + name + "\" is :  " + to_string(bal) + " txcoins.";
    }
    catch (...)
    {
      datagramServerResponse = "Unable to proceed with the transaction as \"" + name + "\" is not part of the network.";
    }
  }
  if (createNewSocket == true)
  {
    // Close Socket
    close(datagram_client_sock);
  }
  return datagramServerResponse;
}

string logTransaction(string sender, string reciever, string amt)
{

  if (amt.empty())
  {
    return "invalid";
  }

  int amount;
  try
  {
    amount = stoi(amt);
  }
  catch (...)
  {
    return "invalid";
  }

  // create Datagram Client and bind to port

  // create UDP socket
  int datagram_client_sock = createUDPSocket();

  // create addressHint for serverM for bind operation
  sockaddr_in datagram_client_hint = createUDPClientAddrHint();

  // bind UDP client to a static port
  if (::bind(datagram_client_sock, (struct sockaddr *)&datagram_client_hint, sizeof(datagram_client_hint)) == -1)
  {
    cerr << "UDP client serverM could bind to port "
         << UDPPORT
         << endl;
    // Close Socket
    close(datagram_client_sock);
    return "invalid";
  }

  string datagramServerResponse;

  // Check if exists Sender, if so return wallet
  string checkWalletSenderResponse = checkWalletQueryServer(1, sender, datagram_client_sock, datagram_client_hint);

  // process UDP response
  if (checkWalletSenderResponse.empty() ||
      checkWalletSenderResponse.compare("empty") == 0 ||
      checkWalletSenderResponse.compare("invalid") == 0)
  {
    datagramServerResponse = "Unable to proceed with the transaction as \"" + sender + "\" is not part of the network.";
    // Check if exists Reciever exists
    string checkWalletRecResponse = checkWalletQueryServer(1, reciever, datagram_client_sock, datagram_client_hint);
    if (checkWalletRecResponse.empty() ||
        checkWalletRecResponse.compare("empty") == 0 ||
        checkWalletRecResponse.compare("invalid") == 0)
    {
      datagramServerResponse = "Unable to proceed with the transaction as \"" + sender + "\" and \"" + reciever + "\" are not part of the network.";
    }
    return datagramServerResponse;
  }
  else
  {
    // Check if exists Reciever exists
    string checkWalletRecResponse = checkWalletQueryServer(1, reciever, datagram_client_sock, datagram_client_hint);
    if (checkWalletRecResponse.empty() ||
        checkWalletRecResponse.compare("empty") == 0 ||
        checkWalletRecResponse.compare("invalid") == 0)
    {
      datagramServerResponse = "Unable to proceed with the transaction as \"" + reciever + "\" is not part of the network.";
      return datagramServerResponse;
    }
    int d;
    try
    {
      d = stoi(checkWalletSenderResponse);
      int bal = 1000 + d;
      if (bal < amount)
      {
        datagramServerResponse = "\"" + sender + "\" was unable to transfer " + amt + " txcoins to \"" + reciever + "\" because of insufficient balance.\n\n";

        // check bal
        datagramServerResponse = datagramServerResponse + checkWallet(sender, false, datagram_client_sock, datagram_client_hint);

        return datagramServerResponse;
      }
    }
    catch (...)
    {
      datagramServerResponse = "\"" + sender + "\" was unable to transfer " + amt + " txcoins to \"" + reciever + "\" because of insufficient balance.\n\n";

      // check bal
      datagramServerResponse = datagramServerResponse + checkWallet(sender, false, datagram_client_sock, datagram_client_hint);

      return datagramServerResponse;
    }
  }

  // sender and reciever exists and wallet has sufficient balance

  // get next transaction number
  int maxTrasNum = -1;
  string maxTran = checkMaxTransNumQueryServer(1, datagram_client_sock, datagram_client_hint);
  if (maxTran.empty() ||
      maxTran.compare("empty") == 0 ||
      maxTran.compare("invalid") == 0)
  {
    datagramServerResponse = "\"" + sender + "\" was unable to transfer " + amt + " txcoins to \"" + reciever + "\" because of error trying to log transaction.";
    return datagramServerResponse;
  }
  try
  {
    maxTrasNum = stoi(maxTran);
  }
  catch (...)
  {
    datagramServerResponse = "\"" + sender + "\" was unable to transfer " + amt + " txcoins to \"" + reciever + "\" because of error trying to log transaction.";
    return datagramServerResponse;
  }
  maxTrasNum = maxTrasNum + 1;

  // randomly select a server TODO
  // srand((unsigned)time(0));
  // randomNumber = rand();
  // int randomServName = (randomNumber % 3) + 1;
  int randomServName = 1;

  // log transaction in file
  datagramServerResponse = logTransInFileQueryServer(randomServName, datagram_client_sock, datagram_client_hint,
                                                     to_string(maxTrasNum), sender, reciever, amt);
  if (datagramServerResponse.empty() ||
      datagramServerResponse.compare("empty") == 0 ||
      datagramServerResponse.compare("invalid") == 0)
  {
    datagramServerResponse = "\"" + sender + "\" was unable to transfer " + amt + " txcoins to \"" + reciever + "\" because of error trying to log transaction.";
    return datagramServerResponse;
  }

  datagramServerResponse = "\"" + sender + "\" successfully transferred " + amt + " txcoins to \"" + reciever + "\".\n\n";

  // check bal
  datagramServerResponse = datagramServerResponse + checkWallet(sender, false, datagram_client_sock, datagram_client_hint);

  // Close Socket
  close(datagram_client_sock);
  return datagramServerResponse;
}

int childFork(int stream_welcoming_sock, int childSocket, sockaddr_in client)
{
  // this is a child process
  // Child socket doesnot need the welcoming socket or listener
  close(stream_welcoming_sock);

  char host[NI_MAXHOST]; // buffer to put client ip - size 1025
  //char svc[NI_MAXSERV];  // buffer to put client port - size 32
  string clientIP;
  string clientPort;

  // Clean up host and svc before populating it
  memset(host, 0, NI_MAXHOST);
  //memset(svc, 0, NI_MAXSERV);

  // get client's IP and port
  inet_ntop(DOMAIN, &client.sin_addr, host, NI_MAXHOST); // convert numeric array to string
  clientPort = to_string(ntohs(client.sin_port));        // ntohs is network to host short
  clientIP = host;
  // cout << clientIP << " connected on " << clientPort << endl;

  // TCP connection is opened, so now exchange messages
  char buf[MAXBUFLEN]; // Buffer to hold the incoming request

  memset(buf, 0, MAXBUFLEN); // Clear the buffer

  // Recieve Request
  int bytesRecv = recv(childSocket, buf, MAXBUFLEN - 1, 0);
  if (bytesRecv == -1)
  {
    cerr << "ServerM : Error recieving message from client " << host << " with host port " << clientPort << endl;
    return -1;
  }

  if (bytesRecv == 0)
  {
    cout << "The client disconnected on ServerM" << endl;
    return -1;
  }

  // Process the input request
  istringstream stringStream(string(buf, 0, bytesRecv));
  vector<string> clientInputs;
  string word;
  string datagramServerResponse;
  int er_flag = 0;

  while (stringStream >> word)
  {
    clientInputs.push_back(word);
  }

  if (clientInputs.size() == 1)
  {
    // Check Wallet requested

    // Display request that was recieved
    cout << "The main server received "
         << "input=\"" + clientInputs[0] + "\" from the client"
         << " using TCP over port "
         << TCPPORT << "." << endl;

    sockaddr_in datagram_client_hint_temp;
    datagramServerResponse = checkWallet(clientInputs[0], true, 0, datagram_client_hint_temp);
    cout << "The main server sent the current balance to the client." << endl;
  }
  else if (clientInputs.size() == 3)
  {
    // Transfer Amount requested

    // Display request that was recieved
    cout << "The main server received "
         << "from \"" << clientInputs[0] << "\""
         << " to transfer " << clientInputs[2] << " coins to \"" << clientInputs[1] << "\""
         << " using TCP over port "
         << TCPPORT << "." << endl;
    datagramServerResponse = logTransaction(clientInputs[0], clientInputs[1], clientInputs[2]);
  }
  else
  {
    // Not a supported request
    cerr << "ServerM : not a supported request by " << host << " with host port " << clientPort << endl;
    datagramServerResponse = "Not a supported Operation!";
    er_flag = -1;
  }

  if (datagramServerResponse.empty())
  {
    datagramServerResponse = "Something went wrong";
    er_flag = -1;
  }

  int n = datagramServerResponse.length();
  char char_array[n + 1];
  strcpy(char_array, datagramServerResponse.c_str());

  // Send response
  if (send(childSocket, char_array, n + 1, 0) == -1)
  {
    cerr << "ServerM : Error sending message from ServerM to " << host << " with host port " << clientPort << endl;
    er_flag = -1;
  }

  // Close child socket
  close(childSocket);
  return er_flag;
}

int main()
{
  // Stream Sock Server (TCP socket server)
  int stream_welcoming_sock = createBindListenStrmSrvrWlcmngSocket();

  if (stream_welcoming_sock == -1)
  {
    return -1;
  }

  // Stream socket is listening successfully
  cout << "The main server is up and running." << endl;

  // client accept loop
  while (1)
  {
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    // Accept or create child socket
    // pull in a request from incoming request queue and create a child socket to process it
    int childSocket = accept(stream_welcoming_sock, (sockaddr *)&client, &clientSize);

    if (childSocket == -1)
    {
      cerr << "Stream socket could not accept client for ServerM";
      continue;
    }

    if (!fork())
    {
      childFork(stream_welcoming_sock, childSocket, client);
      exit(0);
    }

    // Close child socket as parent does not need it
    close(childSocket);
  }

  // Close welcoming socket
  close(stream_welcoming_sock);
  return 0;
}
