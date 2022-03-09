
#include "Networking/sockets.h"
#include "Networking/ServerSocket.h"
#include "Networking/Server.h"

#include <iostream>
#include <pthread.h>
#include <assert.h>


/*
 * Get the client ip number on the socket connection for client i.
 */
void Server::get_ip(int num)
{
  struct sockaddr_storage addr;
  socklen_t len = sizeof addr;  

  getpeername(socket_num[num], (struct sockaddr*)&addr, &len);

  // supports both IPv4 and IPv6:
  char ipstr[INET6_ADDRSTRLEN];  
  if (addr.ss_family == AF_INET) {
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
      inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
  } else { // AF_INET6
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
  }

  names[num]=new octet[512];
  memset(names[num], 0, 512);
  strncpy((char*)names[num], ipstr, INET6_ADDRSTRLEN);

#ifdef DEBUG_NETWORKING
  cerr << "Client IP address: " << names[num] << endl;
#endif
}


void Server::get_name(int num)
{
#ifdef DEBUG_NETWORKING
  cerr << "Player " << num << " started." << endl;
#endif

  // Receive name sent by client (legacy) - not used here
  octet my_name[512];
  receive(socket_num[num],my_name,512);
  receive(socket_num[num],(octet*)&ports[num],4);
#ifdef DEBUG_NETWORKING
  cerr << "Player " << num << " sent (IP for info only) " << my_name << ":"
      << ports[num] << endl;
#endif

  // Get client IP
  get_ip(num);
}


void Server::send_names(int num)
{
  /* Now send the machine names back to each client 
   * and the number of machines
   */
  send(socket_num[num],nmachines,4);
  for (int i=0; i<nmachines; i++)
    {
      send(socket_num[num],names[i],512);
      send(socket_num[num],(octet*)&ports[i],4);
    }
}


/* Takes command line arguments of 
       - Number of machines connecting
       - Base PORTNUM address
*/

Server::Server(int argc,char **argv)
{
  if (argc != 3)
    { cerr << "Call using\n\t";
      cerr << "Server.x n PortnumBase\n";
      cerr << "\t\t n           = Number of machines" << endl;
      cerr << "\t\t PortnumBase = Base Portnum\n";
      exit(1);
    }
  nmachines=atoi(argv[1]);
  PortnumBase=atoi(argv[2]);
}

Server::Server(int nmachines, int PortnumBase) :
    nmachines(nmachines), PortnumBase(PortnumBase)
{
}

void Server::start()
{
  int i;

  names.resize(nmachines);
  ports.resize(nmachines);

  /* Set up the sockets */
  socket_num.resize(nmachines);
  for (i=0; i<nmachines; i++) { socket_num[i]=-1; }

  // port number one lower to avoid conflict with players
  ServerSocket server(PortnumBase - 1);
  server.init();

  // set up connections
  for (i=0; i<nmachines; i++)
    {
#ifdef DEBUG_NETWORKING
      cerr << "Waiting for player " << i << endl;
#endif
      socket_num[i] = server.get_connection_socket("P" + to_string(i));
#ifdef DEBUG_NETWORKING
      cerr << "Connected to player " << i << endl;
#endif
    }

  // get names
  for (i=0; i<nmachines; i++)  
    get_name(i);  

  // check setup, party 0 doesn't matter
  bool all_on_local = true, none_on_local = true;
  for (i = 1; i < nmachines; i++)
    {
      bool on_local = string((char*)names[i]).compare("127.0.0.1");
      all_on_local &= on_local;
      none_on_local &= not on_local;
    }
  if (not all_on_local and not none_on_local)
    {
      cout << "You cannot address Server.x by localhost if using different hosts" << endl;
      exit(1);
    }

  // send names
  for (i=0; i<nmachines; i++)
    send_names(i);

  for (i=0; i<nmachines; i++) 
    { delete[] names[i]; }

  for (int i = 0; i < nmachines; i++)
    close(socket_num[i]);
}

void* Server::start_in_thread(void* server)
{
  ((Server*)server)->start();
  return 0;
}

Server* Server::start_networking(Names& N, int my_num, int nplayers,
        string hostname, int portnum, int my_port)
{
#ifdef DEBUG_NETWORKING
  cerr << "Starting networking for " << my_num << "/" << nplayers
      << " with server on " << hostname << ":" << (portnum - 1) << endl;
#endif
  assert(my_num >= 0);
  assert(my_num < nplayers);
  Server* server = 0;
  pthread_t thread;
  if (my_num == 0)
    {
      pthread_create(&thread, 0, Server::start_in_thread,
          server = new Server(nplayers, portnum));
    }
  N.init(my_num, portnum, my_port, hostname.c_str());
  if (my_num == 0)
    {
      pthread_join(thread, 0);
      delete server;
    }
  return 0;
}
