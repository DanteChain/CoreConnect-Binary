#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <tcp/h/tcp_error.hpp>
#include <logger/h/logger.hpp>





class tcp_client;
class tcp_server;


class tcp_connection
    {
public:
    friend class tcp_client;                                                        // friend class can construct this class
    friend class tcp_server;                                                        // friend class can construct this class
    ~tcp_connection();                                                              // destructor
    enum connect_mode                                                               // mode connection is going to be in
        {                                                                           // wait or no-wait mode
        synchronous,
        asynchronous
        };
    typedef struct
        {
        int         bytes_rxed;
        int         bytes_txed;
        } TCP_CONNECTION_INFO;
    int  close();                                                                   // close the tcp connection
    int get_fd();                                                                   // get tcp file descriptor
    std::string get_local_host_name();                                              // get the local host name if any
    std::string get_local_host_name_ip();                                           // get local host ip addr as nnn.nnn.nnn.nnn
    std::string get_local_port_name();                                              // get the local port name if any
    int get_local_port_number();                                                    // get local port as a number (int)
    enum connect_mode get_mode();                                                   // get current connect mode
    std::string get_remote_host_name();                                             // get remote host name if any
    std::string get_remote_host_name_ip();                                          // get local host ip addr as nnn.nnn.nnn.nnn
    std::string get_remote_port_name();                                             // get remote port name if any
    int get_remote_port_number();                                                   // get remote port as a number (int)
    TCP_CONNECTION_INFO get_statistics();                                           // get current connection statistics
    int read_data(void *data, int max_len, int &actual_len);                        // read data from the connection
    int set_mode(enum connect_mode mode);                                           // set the new connect mode
    void set_logger(logger *plog);                                                  // set a ptr to an optional logger object
    int write_data(const void *data, int len, int &actual_len);                     // write data to the connection
    int get_rx_buffer_size();
    int get_tx_buffer_size();
    bool SetLingerOption (bool bEnabled);                                           // Enable/disable lingering, returns true when successful, false otherwise

protected:
    int  p_clear_flags(int fd, int flags);                                          // clear the bit encoded file flags
    int p_set_flags(int fd, int flags);                                             // set the bit encoded file flags
    void p_set_fd(int fd);                                                          // setup the connection file descriptor
    int p_set_local_port(const std::string &port);                                  // set the local port
    int p_set_remote_host(const std::string &host);                                 // set the remote host
    int p_set_remote_port(const std::string &port);                                 // set the remote port
    int p_set_socket_options();                                                     // set standard connection options
    int p_get_rx_buffer_size();
    int p_get_tx_buffer_size();
private:
    tcp_connection(int sz_rx_buf = 8192, int sz_tx_buf = 8192);                     // constructor must be private
    tcp_connection(const tcp_connection &);                                         // copy constructor, dont implement
    tcp_connection &operator=(const tcp_connection &);                              // implement an assignment op
    int pri_set_host_info(const std::string &host, std::string &host_name, struct in_addr &host_ip);// set host information up
    int pri_set_port_info(const std::string &port, std::string &port_name, int &port_number);       // set port information up
    static pthread_mutex_t          m_lock;                                         // lock to protect from bugs in linux libc
    int                             m_fd;                                           // connection file descriptor
    TCP_CONNECTION_INFO             m_stats;
    std::string                     m_local_host_name;                              // saved remote host name
    struct in_addr                  m_local_host_ip;                                // saved remote host ip address (binary)
    std::string                     m_local_port_name;                              // local port name
    int                             m_local_port_number;                            // local port number
    std::string                     m_remote_host_name;                             // saved remote host name
    struct in_addr                  m_remote_host_ip;                               // saved remote host ip address (binary)
    std::string                     m_remote_port_name;                             // remote port name
    int                             m_remote_port_number;                           // remote port number
    logger                          *m_plog;                                        // ptr to optional logger object
    enum connect_mode               m_mode;                                         // mode of connection
    int                             m_sz_tx_buf;
    int                             m_sz_rx_buf;
    };

