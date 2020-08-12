#pragma once

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <logger/h/logger.hpp>
#include <tcp/h/tcp_connection.hpp>


using namespace std;



/**
 * class enables an application to create an active connection to a tcp server.
 * The calls can operate in wait or no-wait mode, but in no-wait mode the application
 * is responsible for knowing when to test whether a connection attempt has succeeded
 * or not. Once a connection has been successful a tcp_connection object will be newed
 * and given to the application. It is that object that controls the connection. The
 * tcp_client object may then be destroyed or used to make another connection.
*/
class tcp_client : public tcp_connection
    {
public:
    tcp_client(int sz_rx_buf = 8192, int sz_tx_buf = 8192);                 ///< constructor
    ~tcp_client();                                                          ///< destructor
    int connect(tcp_connection **p);                                        ///< connect a tcp connection
    int set_remote_host(const string &host);                                ///< set the host to connect to
    int set_remote_port(const string &port);                                ///< set the port to connect to
    int set_timeout(int timeout);                                           ///< set the optional wait mode timeout in secs
private:
    tcp_client(const tcp_client &);                                         ///< copy constructor, dont implement
    tcp_client &operator=(const tcp_client &);                              ///< assignment operator, dont implement
    int p_connect_block();                                                  ///< connect in wait mode
    int p_connect_non_block();                                              ///< connect in no wait mode
    void p_copy_tcp_connection_info(tcp_connection **p);                    ///< copy tcp_connection object for application
    void p_set_connect_addr();                                              ///< set the remote connect address
    void p_set_local_port_info();                                           ///< setup the local port info once connected ok
    int                                 m_timeout;                          ///< blocked connect timeout in secs
    struct sockaddr_in                  m_connect_addr;                     ///< address to connect to
    };


