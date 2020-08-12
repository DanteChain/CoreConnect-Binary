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
#include <netinet/in.h>
#include <h/tcp_connection.hpp>
#include <h/tcp_client.hpp>




// constructor. Creates a tcp client object ready to use.

tcp_client::tcp_client(int sz_rx_buf, int sz_tx_buf) : tcp_connection(sz_rx_buf, sz_tx_buf)
    {
    m_timeout = 0;
    memset(&m_connect_addr, 0, sizeof(m_connect_addr));
    return;
    }



// destructor. Destroys the client tcp object, but first closing any existing connection.

tcp_client::~tcp_client()
    {
    close();
    return;
    }



// function attempts to connect the tcp client to a remote tcp host/port specified by other
// member functions. If the connection is successful a tcp_connection object is created and
// a pointer to it is handed back to the caller. It is the callers responsibility to delete
// this object when finished with it. Once connect has been called and returns successfully
// a further call will attempt to create another new connection.
// Any error will be returned to the caller.

int tcp_client::connect(tcp_connection **p)
    {
    int ret;

    ret = (m_mode == synchronous) ? p_connect_block() : p_connect_non_block();
    if (ret == E_tcp_no_error)
        {
        p_copy_tcp_connection_info(p);
        m_fd = -1;
        }
    return (ret);
    }





// function sets the connect timeout in seconds. The timeout is only used for a blocking
// connection attempt. The caller is responsible for any non blocking connect timeouts and
// how they are implemented. Any error will be returned to the caller.

int tcp_client::set_timeout(int timeout)
    {
    int ret = E_tcp_no_error;

    if (get_fd() != -1)
        ret = E_tcp_already_connected;
    else if (timeout <= 0)
        m_timeout = 0;
    else
        m_timeout = timeout;
    return (ret);
    }



// function sets the remote host information. This may be a host name or ip address.
// Any error will be returned to the caller.

int tcp_client::set_remote_host(const string &host)
    {
    return (p_set_remote_host(host));
    }



// function sets the remote port information. This may be a service name or port number.
// Any error will be returned to the caller.

int tcp_client::set_remote_port(const string &port)
    {
    return (p_set_remote_port(port));
    }



// function attempts to do a blocked connect to the remote end. First the connect
// address is setup and a socket opened, then standard socket options are set and
// the fd placed in no-wait mode. A connect is performed and then the function waits
// on select for the result. This is where the timeout is implemented.
// select will either return sucess, a timeout or an error. Any error will close the
// connection but if all ok, the connection is returned to wait mode, and the local
// port information is setup and the function returns.
// Any error will be returned to the caller.

int tcp_client::p_connect_block()
    {
    fd_set rset, wset;
    struct timeval tval;
    int n, value;
    int ret = E_tcp_no_error, ret1;
    socklen_t ret_len;

    p_set_connect_addr();
    if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        ret = E_tcp_socket;
    else if ((ret = p_set_socket_options()) != E_tcp_no_error)
        ;
    else if ((ret = p_set_flags(m_fd, O_NDELAY)) != E_tcp_no_error)
        ;
    else if ((n = ::connect(m_fd, (struct sockaddr *)&m_connect_addr, sizeof(m_connect_addr))) == 0)
        ;
    else if (n == -1 && errno != EINPROGRESS)
        ret = E_tcp_connect_failed;
    else
        {
        FD_ZERO(&rset);
        FD_SET(m_fd, &rset);
        wset = rset;
        tval.tv_sec = m_timeout;
        tval.tv_usec = 0;
        if ((n = select(m_fd + 1, &rset, &wset, NULL, m_timeout ? &tval : NULL)) == -1)
            ret = E_tcp_select_failed;
        else if (n == 0)
            ret = E_tcp_connect_timeout;
        else if (FD_ISSET(m_fd, &rset) || FD_ISSET(m_fd, &wset))
            {
            value = 0;
            ret_len = sizeof(value);
            if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &value, &ret_len) == -1 || value != 0)
                ret = E_tcp_connect_error;
            }
        else
            ret = E_tcp_select_failed;
        }
    ret1 = p_clear_flags(m_fd, O_NDELAY);
    if (ret != E_tcp_no_error)
        close();
    if (ret == E_tcp_no_error && ret1 == E_tcp_no_error)
        p_set_local_port_info();
    return (ret == E_tcp_no_error ? ret1 : ret);
    }







// function attempts to do a non blocked connect to the remote end. This function is called
// repeatedly until it either fails or succeeds. First time around the connect address is setup,
// the socket opened, options set, and the fd placd in no-wait mode. Then as well as on all
// subsequent calls, connect is called. Connect can return success, in progress or failure.
// For each case an error code will be returned. On error the connection will be closed.
// Any error will be returned to the caller.

int tcp_client::p_connect_non_block()
    {
    int ret = E_tcp_no_error;

    if (get_fd() == -1)
        {
        p_set_connect_addr();
        if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            ret = E_tcp_socket;
        else if ((ret = p_set_socket_options()) != E_tcp_no_error)
            ;
        else if ((ret = p_set_flags(m_fd, O_NDELAY)) != E_tcp_no_error)
            ;
        }
    if (ret == E_tcp_no_error)
        {
        if (::connect(m_fd, (struct sockaddr *)&m_connect_addr, sizeof(m_connect_addr)) != 0)
            {
            if (errno == EISCONN)
                ret = E_tcp_no_error;
            else if (errno == EINPROGRESS || errno == EAGAIN)
                ret = E_tcp_conn_in_progress;
            else
                ret = E_tcp_connect_failed;
            }
        }
    if (ret == E_tcp_no_error)
        p_set_local_port_info();
    if (ret != E_tcp_no_error && ret != E_tcp_conn_in_progress)
        close();
    return (ret);
    }



// private function creates a new tcp_connection object, populates it and
// hands a pointer to the object back to the caller. It is the callers
// responsibility to delete this object when finished with it.

void tcp_client::p_copy_tcp_connection_info(tcp_connection **p)
    {
    *p = new tcp_connection;
    **p = *(tcp_connection *)this;
    return;
    }






// function sets the connect address information up for the connect function
// to use.
// MDW 18/4/2003. GNU provides a horrible macro for htons(). This macro does not follow the
// syntax rules of C so the Intel compiler gets a bit pissed off trying to compile it. The annoying
// thing is that the Intel compiler knows its gcc being turdy but still complains. Anyway to force
// the function call rather than the macro place () around the function name. This keeps both compilers
// very happy. The performance hit is 0 considering when and how this is called.

void tcp_client::p_set_connect_addr()
    {
    memset(&m_connect_addr, 0, sizeof(m_connect_addr));
    m_connect_addr.sin_family = AF_INET;
    m_connect_addr.sin_port = (htons)(get_remote_port_number());
    memcpy(&m_connect_addr.sin_addr, &m_remote_host_ip.s_addr, sizeof(m_remote_host_ip.s_addr));
    return;
    }




// function sets up the local port information once a connection has been made.
// Basically the ephemeral port number is determined and saved.

void tcp_client::p_set_local_port_info()
    {
    struct sockaddr_in tmp;
    socklen_t tmp_len = sizeof(tmp);
    char buf[100];
    string port_num;

    if (getsockname(m_fd, (struct sockaddr *)&tmp, &tmp_len) == 0)
        {
        sprintf(buf, "%d", (ntohs)(tmp.sin_port));
        port_num = buf;
        p_set_local_port(port_num);
        }
    return;
    }

