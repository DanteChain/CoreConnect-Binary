#pragma once

#include <pthread.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <logger/h/logger.hpp>
#include <tcp/h/tcp_connection.hpp>


using namespace std;




// class enables an application to listen for a connection on a well known port.
// The calls can operate in wait or no-wait mode, but in no-wait mode the application
// is responsible for knowing when to test whether a connection attempt has suceeded
// or not. Once a connection has been sucessfull a tcp_connection object will be newed
// and given to the application. It is that object that controls the connection. The
// tcp_server object may then be destroyed or used to listen for another connection.

class tcp_server : public tcp_connection
    {
public:
    tcp_server(int sz_rx_buf = 8192, int sz_tx_buf = 8192);         // constructor
    ~tcp_server();                                                  // destructor
    int listen(tcp_connection **p);                                 // listen for a client connection
    void set_backlog(unsigned int backlog = 1);                     // set the port to listen on
    int set_listen_port(const string &port);                        // set the port to connect to
private:
    tcp_server(const tcp_server &);                                 // copy constructor, dont implement
    tcp_server &operator=(const tcp_server &);                      // assignment operator, dont implement
    void p_copy_tcp_connection_info(tcp_connection **p);            // copy tcp_connection object for application
    void p_set_remote_host_port_info();                             // set the remote port info once a connection is made
    int p_set_listen_addr();                                        // set the listen address info up
    int p_listen_block();                                           // blocking listen for a connection
    int p_listen_non_block();                                       // non-blocking listen for a connection
    struct sockaddr_in      m_listen_addr;                          // listen address
    int                     m_accept_fd;                            // tmp place for accepted connection fd
    unsigned int            m_backlog;                              // listen backlog 1 - 200 lets say
    };



