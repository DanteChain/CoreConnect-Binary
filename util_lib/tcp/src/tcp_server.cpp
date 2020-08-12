#include <pthread.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cctype>
#include <h/tcp_connection.hpp>
#include <h/tcp_server.hpp>




// constructor. Creates a tcp server object ready to use.

tcp_server::tcp_server(int sz_rx_buf, int sz_tx_buf) : tcp_connection(sz_rx_buf, sz_tx_buf)
    {
    memset(&m_listen_addr, 0, sizeof(m_listen_addr));
    m_accept_fd = -1;
    m_backlog = 1;
    return;
    }



// destructor. Destroys the server tcp object, but first closing any existing connection.

tcp_server::~tcp_server()
    {
    close();
    return;
    }




// function sets the backlog parameter for the listen system call. An arbritary limit has been
// placed on the value. Outside of this range the backlog is left unchanged.
// The function does not return anything.

void tcp_server::set_backlog(unsigned int backlog)
    {
    if (m_fd == -1 && backlog > 0 && backlog < 200)
        m_backlog = backlog;
    return;
    }




// function attempts to connect the tcp server to a remote tcp host/port specified by other
// member functions. If the connection is successful a tcp_connection object is created and
// a pointer to it is handed back to the caller. It is the callers responsibility to delete
// this object when finished with it. Once connect has been called and returns successfully
// a further call will attempt to create another new connection.
// Any error will be returned to the caller.

int tcp_server::listen(tcp_connection **p)
    {
    int ret;

    ret = (m_mode == synchronous) ? p_listen_block() : p_listen_non_block();
    if (ret == E_tcp_no_error)
        {
        p_copy_tcp_connection_info(p);
        (*p)->p_set_fd(m_accept_fd);
        m_accept_fd = -1;
        }
    return (ret);
    }





// function attempts to listen for an incomming connection in blocked mode. First the socket
// is opened, then the listen address is created and the standard socket options set. A listen
// is executed with the specified backlog and then it waits in accept for the first connection.
// If successful the remote address information is setup and the function returns, otherwise the
// listen port is closed before returning an error.

int tcp_server::p_listen_block()
    {
    int ret = E_tcp_no_error;

    if (m_fd == -1)
        {
        if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            ret = E_tcp_socket;
        else if ((ret = p_set_listen_addr()) != E_tcp_no_error)
            ;
        else if ((ret = p_set_socket_options()) != E_tcp_no_error)
            ;
        else if (::bind(m_fd, (struct sockaddr *)&m_listen_addr, sizeof(m_listen_addr)) != 0)
            ret = E_tcp_bind_failed;
        else if (::listen(m_fd, m_backlog) != 0)
            ret = E_tcp_listen_fail;
        }
    if (ret == E_tcp_no_error)
        {
        if ((m_accept_fd = ::accept(m_fd, NULL, NULL)) == -1)
            ret = E_tcp_accept_failed;
        else
            p_set_remote_host_port_info();
        }
    if (ret != E_tcp_no_error)
        close();
    return (ret);
    }






// function attempts to listen for an incomming connection in non blocked mode. First the socket
// is opened, then the listen address is created and the standard socket options set. A listen
// is executed with the specified backlog and then accept is called to see is a connection is
// waiting. This call returns immediately setting the errno to indicate whether an error has
// occurred or a connection is pending or there the listen is just still in progress.
// The current status will be returned to the caller. Once a connection is successfull then the
// remote address information is setup for reference.

int tcp_server::p_listen_non_block()
    {
    int ret = E_tcp_no_error;

    if (m_fd == -1)
        {
        if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            ret = E_tcp_socket;
        else if ((ret = p_set_listen_addr()) != E_tcp_no_error)
            ;
        else if ((ret = p_set_socket_options()) != E_tcp_no_error)
            ;
        else if ((ret = p_set_flags(m_fd, O_NDELAY)) != E_tcp_no_error)
            ;
        else if (::bind(m_fd, (struct sockaddr *)&m_listen_addr, sizeof(m_listen_addr)) != 0)
            ret = E_tcp_bind_failed;
        else if (::listen(m_fd, m_backlog) != 0)
            ret = E_tcp_listen_fail;
        }
    if (ret == E_tcp_no_error)
        {
        if ((m_accept_fd = ::accept(m_fd, NULL, NULL)) > 0)
            ;
        else if (errno == EWOULDBLOCK)
            ret = E_tcp_listen_in_progress;
        else
            ret = E_tcp_accept_failed;
        }
    if (ret == E_tcp_no_error)
        p_set_remote_host_port_info();
    if (ret != E_tcp_no_error && ret != E_tcp_listen_in_progress)
        close();
    return (ret);
    }






// function sets up the listen address. In this case the port and the interface to
// listen on. The listen interface is always set to any address.
// The function always returns success.
// MDW 18/4/2003. GNU provides a horrible macro for htons() and htonl(). These macros do not follow the
// syntax rules of C so the Intel compiler gets a bit pissed off trying to compile it. The annoying
// thing is that the Intel compiler knows its gcc being turdy but still complains. Anyway to force
// the function call rather than the macro place () around the function names. This keeps both compilers
// very happy. The performance hit is 0 considering when and how this is called.

int tcp_server::p_set_listen_addr()
    {
    memset(&m_listen_addr, 0, sizeof(m_listen_addr));
    m_listen_addr.sin_family = AF_INET;
    m_listen_addr.sin_addr.s_addr = (htonl)(INADDR_ANY);
    m_listen_addr.sin_port = (htons)(get_local_port_number());
    return (E_tcp_no_error);
    }





// function copies the current connection information over to a new tcp_connection
// object that is handed back to the application to use to control the connection.

void tcp_server::p_copy_tcp_connection_info(tcp_connection **p)
    {
    *p = new tcp_connection;
    **p = *(tcp_connection *)this;
    return;
    }




// function sets the listen well known port up.
// any error is returned to the caller.

int tcp_server::set_listen_port(const string &port)
    {
    return (p_set_local_port(port));
    }




// function sets up the remote host port information once a connection has been made.
// This information is for application reference only.
// Function does return any value.

void tcp_server::p_set_remote_host_port_info()
    {
    struct sockaddr_in tmp;
    socklen_t tmp_len = sizeof(tmp);
    char buf[256];
    string port_num, host_name;

    if (getpeername(m_accept_fd, (struct sockaddr *)&tmp, &tmp_len) == 0)
        {
        inet_ntop(AF_INET, &tmp.sin_addr, buf, sizeof(buf));
        host_name = buf;
        p_set_remote_host(host_name);
        sprintf(buf, "%d", (ntohs)(tmp.sin_port));
        port_num = buf;
        p_set_remote_port(port_num);
        }
    return;
    }

