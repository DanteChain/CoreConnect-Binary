#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <h/tcp_connection.hpp>




    using namespace std;
    pthread_mutex_t tcp_connection::m_lock = PTHREAD_MUTEX_INITIALIZER;                 // lock to protect from bugs in linux libc

    std::map<tcp_errors, std::string> TCPErrorStrings::m_MessageText;
    TCPErrorStrings::Initialiser TCPErrorStrings::m_Initialiser;

// constructor creates a tcp_connection object and initialises it ready foruse.
// All members are defaulted except for the local host name and host ip address.
// The constructor is private so only the friend classes are allowed to construct
// such an object. Applications can only use this object that has been created from
// a tcp_client or tcp_server correctly making a connection.

tcp_connection::tcp_connection(int sz_rx_buf, int sz_tx_buf)
    {
        char tmp[256 + 1];

        m_fd = -1;
        m_stats.bytes_txed = 0;
        m_stats.bytes_rxed = 0;
        m_local_host_name = "";
        memset(&m_local_host_ip, 0, sizeof(m_local_host_ip));
        m_local_port_name = "";
        m_local_port_number = -1;
        m_remote_host_name = "";
        memset(&m_remote_host_ip, 0, sizeof(m_remote_host_ip));
        m_remote_port_name = "";
        m_remote_port_number = -1;
        m_plog = 0;
        m_mode = synchronous;
        if (gethostname(tmp, sizeof(tmp) - 1) != 1)
            pri_set_host_info(string(tmp), m_local_host_name, m_local_host_ip);
        m_sz_rx_buf = (sz_rx_buf < 1024 || sz_rx_buf > 8388608) ? 1024 : sz_rx_buf;
        m_sz_tx_buf = (sz_tx_buf < 1024 || sz_tx_buf > 8388608) ? 1024 : sz_tx_buf;
    return;
    }




// destructor closes any existing connection and destroys the object.

tcp_connection::~tcp_connection()
    {
    close();
    return;
    }



// assigment operator. This is used to duplicate the tcp_connection information.
// A pointer to the duplicated object is returned back to the caller and it is this
// object that will be used by the application to control the tcp connection.
// The application must delete this object when finished with it.

tcp_connection &tcp_connection::operator=(const tcp_connection &rhs)
    {
    m_fd = rhs.m_fd;
    m_stats = rhs.m_stats;
    m_local_host_name = rhs.m_local_host_name;
    m_local_host_ip = rhs.m_local_host_ip;
    m_local_port_name = rhs.m_local_port_name;
    m_local_port_number = rhs.m_local_port_number;
    m_remote_host_name = rhs.m_remote_host_name;
    m_remote_host_ip = rhs.m_remote_host_ip;
    m_remote_port_name = rhs.m_remote_port_name;
    m_remote_port_number = rhs.m_remote_port_number;
    m_plog = rhs.m_plog;
    m_mode = rhs.m_mode;
    m_sz_rx_buf = rhs.m_sz_rx_buf;
    m_sz_tx_buf = rhs.m_sz_tx_buf;
    return (*this);
    }




// function closes the tcp connection if it is not already closed.
// The file descriptor is set to -1 to show its idle. Any error will
// be returned to the caller.

int tcp_connection::close()
    {
    int ret;

    if (m_fd == -1)
        ret = E_tcp_already_closed;
    else if (::close(m_fd) != 0)
        ret = E_tcp_close_fail;
    else
        {
        m_fd = -1;
        ret = E_tcp_no_error;
        }
    return (ret);
    }



// function just returns the objects current file descriptor. This is used
// when wanting to manipulate flags or the state of the connection.

int tcp_connection::get_fd()
    {
    return (m_fd);
    }


// function returns the local host name

string tcp_connection::get_local_host_name()
    {
    return (m_local_host_name);
    }



// function returns the local host name as a dotted decimal network ip address

string tcp_connection::get_local_host_name_ip()
    {
    return (string(inet_ntoa(m_local_host_ip)));
    }



// function returns the local port name if any has been defined

string tcp_connection::get_local_port_name()
    {
    return (m_local_port_name);
    }



// function returns the local port number

int tcp_connection::get_local_port_number()
    {
    return (m_local_port_number);
    }



// function returns the remote host name

string tcp_connection::get_remote_host_name()
    {
    return (m_remote_host_name);
    }



// function returns the remote host name as a dotted decimal network ip address

string tcp_connection::get_remote_host_name_ip()
    {
    return (string(inet_ntoa(m_remote_host_ip)));
    }



// function returns the local port name if any has been defined

string tcp_connection::get_remote_port_name()
    {
    return (m_remote_port_name);
    }



// function returns the local port number

int tcp_connection::get_remote_port_number()
    {
    return (m_remote_port_number);
    }



// function returns a structure containing data transfer statistics
// concerned with the tcp connection.

tcp_connection::TCP_CONNECTION_INFO tcp_connection::get_statistics()
    {
    return (m_stats);
    }




int tcp_connection::get_rx_buffer_size()
    {
    return (p_get_rx_buffer_size());
    }


int tcp_connection::get_tx_buffer_size()
    {
    return (p_get_tx_buffer_size());
    }






// function reads data from the tcp connection. First the parameters are
// checked for correctness, then the read is performed of up to max_len
// bytes. The function will place the actual number of bytes in the
// supplied parameter. Any error will be reported to the caller including
// calls that would block because there is no data to read in asynchronous
// mode.

int tcp_connection::read_data(void *data, int max_len, int &actual_len)
    {
    int read_len;
    int ret = E_tcp_no_error;

    actual_len = 0;
    if (data == 0)
        ret = E_tcp_null_ptr;
    else if (max_len <= 0)
        ret = E_tcp_max_read_len;
    else if (m_fd == -1)
        ret = E_tcp_already_closed;
    else if ((read_len = read(m_fd, data, max_len)) > 0)
        {
        m_stats.bytes_rxed += read_len;
        actual_len = read_len;
        ret = E_tcp_no_error;
        }
    else if (read_len == 0)
        ret = E_tcp_read_fail;
    else if (read_len == -1)
        {
        if (errno == EINTR)
            ret = E_tcp_no_error;
        else if (errno == EAGAIN)
            ret = E_tcp_would_block;
        else
            ret = E_tcp_read_error;
        }
    return (ret);
    }





// function sets the connection mode for the object. synchronous
// is wait mode where read/write calls may block and asynchronous
// is no-wait mode where these calls will return error codes if they
// would block. Any error is returned to the caller.

int tcp_connection::set_mode(enum connect_mode mode)
    {
    int ret;

    m_mode = mode;
    if (m_fd == -1)
        ret = E_tcp_no_error;
    else if (m_mode == asynchronous)
        ret = p_set_flags(m_fd, O_NDELAY);
    else
        ret = p_clear_flags(m_fd, O_NDELAY);
    return (ret);
    }




// function saves a pointer to a logger object and from that point on will
// use it internally to log various bits of information. The logger object may
// be changed at any time.

void tcp_connection::set_logger(logger *plog)
    {
    m_plog = plog;
    return;
    }




// function attempts to write data to the tcp connection. First the args are checked
// for correctness and if ok, the data is written to the connection. The caller is told
// via an arg how bytes have been written. (This can be zero bytes). Any interrupted
// system calls are ignored, but any other error will be returned to the caller.  In wait
// mode the function may block forever waiting to transmit data. In no wait mode if a call
// to write would block an error will be returned to indicate this. A successful write will
// update the statistics data.

int tcp_connection::write_data(const void *data, int len, int &actual_len)
    {
    char *src = (char *)data;
    int ret = E_tcp_no_error;
    int write_len;

    actual_len = 0;
    if (src == 0)
        ret = E_tcp_null_ptr;
    else if (len <= 0)
        ret = E_tcp_max_write_len;
    else if (m_fd == -1)
        ret = E_tcp_already_closed;
    while (ret == E_tcp_no_error && len)
        {
        if ((write_len = write(m_fd, src, len)) > 0)
            {
            m_stats.bytes_txed += write_len;
            actual_len += write_len;
            src += write_len;
            len -= write_len;
            }
        else if (errno == EINTR)
            ;
        else if (errno == EAGAIN)
            ret = E_tcp_would_block;
        else
            ret = E_tcp_write_fail;
        }
    return (ret);
    }





// protected function attempts to clear the specified flags for the given fd.
// Any error will be returned to the caller.

int tcp_connection::p_clear_flags(int fd, int flags)
    {
    int val;
    int ret;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        ret = E_tcp_get_flags;
    else if (val &= ~flags, fcntl(fd, F_SETFL, val) < 0)
        ret = E_tcp_clear_flags;
    else
        ret = E_tcp_no_error;
    return (ret);
    }




// protected function returns the current connection file descriptor. If the connection
// has failed and has been closed it will be set to -1.

void tcp_connection::p_set_fd(int fd)
    {
    m_fd = fd;
    return;
    }




// protected function attempts to set the specified flags for the given fd.
// Any error will be returned to the caller.

int tcp_connection::p_set_flags(int fd, int flags)
    {
    int ret;
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        ret = E_tcp_get_flags;
    else if (val |= flags, fcntl(fd, F_SETFL, val) < 0)
        ret = E_tcp_set_flags;
    else
        ret = E_tcp_no_error;
    return (ret);
    }



// protected function sets the local port information for the connection.

int tcp_connection::p_set_local_port(const string &port)
    {
    return (pri_set_port_info(port, m_local_port_name, m_local_port_number));
    }




// protected function sets the remote host information for the connection.

int tcp_connection::p_set_remote_host(const string &host)
    {
    return (pri_set_host_info(host, m_remote_host_name, m_remote_host_ip));
    }




// protected function sets the remote port information for the connection.

int tcp_connection::p_set_remote_port(const string &port)
    {
    return (pri_set_port_info(port, m_remote_port_name, m_remote_port_number));
    }




// protected function sets the local port information for the connection.

int tcp_connection::p_set_socket_options()
    {
    int val1 = 1, val2 = 0, ret;

    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&val1, sizeof(val1)) != 0)
        ret = E_tcp_set_sock_reuse;
    else if (!SetLingerOption(true))
        ret = E_tcp_set_sock_linger;
    else if (setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&val1, sizeof(val1)) != 0)
        ret = E_tcp_set_sock_kalive;
    else if (setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&val1, sizeof(val1)) != 0)
        ret = E_tcp_set_disable_nagle;

    else if (setsockopt(m_fd, IPPROTO_TCP, TCP_CORK, (char *)&val2, sizeof(val2)) != 0)
        ret = E_tcp_set_disable_cork;

//    else if (setsockopt(m_fd, IPPROTO_TCP, TCP_QUICKACK, (char *)&val1, sizeof(val1)) != 0)                     // MDW test only for xetra
//        ret = E_tcp_set_disable_nagle;
    else if (setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (char *)&m_sz_tx_buf, sizeof(m_sz_tx_buf)) != 0)
        ret = E_tcp_set_sock_tx_buf;
    else if (setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&m_sz_rx_buf, sizeof(m_sz_rx_buf)) != 0)
        ret = E_tcp_set_sock_rx_buf;


//    else if (getsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&rx_num, (socklen_t *)&val2) != 0)   //       MDW for testing only
//        ret = (int)8000;
//
//    else if (getsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (char *)&tx_num, (socklen_t *)&val2) != 0)   //       MDW for testing only
//        ret = (int)8000;

    else
        ret = E_tcp_no_error;
    return (ret);
    }







/**
 * The member l_onoff acts as a Boolean value, where a nonzero value indicates TRUE and zero indicates FALSE.
 * The three variations of this option are specified as follows:
 *
 * Setting l_onoff to FALSE causes member l_linger to be ignored and the default close(2) behaviour implied.
 * That is, the close(2) call will return immediately to the caller, and any pending data will be delivered if possible.
 *
 * Setting l_onoff to TRUE causes the value of member l_linger to be significant. When l_linger is nonzero,
 * this represents the time in seconds for the timeout period to be applied at close(2) time (the close(2)
 * call will "linger"). If the pending data and successful close occur before the timeout occurs, a successful
 * return takes place. Otherwise, an error return occur and errno is set to the value of EWOULDBLOCK.
 *
 * Setting l_onoff to TRUE and setting l_linger to zero causes the connection to be aborted and any pending
 * data is immediately discarded upon close(2).
 * @param bEnabled True will set linger to the default values (on), false will set it to 0 seconds (off)
 * @return true if the operation is successful, false other wise.
 * @note When linger is disabled and the socket is closed the client will receive a tcp exception rather than a
 * read error
 */
bool tcp_connection::SetLingerOption (bool bEnabled)
    {
    if (m_fd == -1)
        {
        return false;
        }
    struct linger lin;
    lin.l_onoff = (bEnabled) ? 0 : 1;                           // 1 => Use the linger value
    lin.l_linger = 0;                                           // When onoff is 1, 0 will turn linger off, otherwise it is ignored
    return setsockopt(m_fd, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin)) == 0;
    }





int tcp_connection::p_get_rx_buffer_size()
    {
    int val2 = 4, num = 0;

    if (getsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&num, (socklen_t *)&val2) != 0)
        return (0);
    return (num);
    }


int tcp_connection::p_get_tx_buffer_size()
    {
    int val2 = 4, num = 0;

    if (getsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (char *)&num, (socklen_t *)&val2) != 0)
        return (0);
    return (num);
    }







// private function given a host name or ip address looks up this information and fills in
// the supplied arguments. It is legal to have a ip address that has no host name, but not
// the other way round. Note the use of reentrant functions to be multi-thread safe. (_r).
// Any error will be returned to the caller.

int tcp_connection::pri_set_host_info(const string &host, string &host_name, struct in_addr &host_ip)
    {
    int ret = E_tcp_no_error;
    int error;
    struct hostent he, *ptr;
    char buf[1024];

//    std::cout << "finding out about host: " << host << std::endl;
    host_name = "";
    memset(&host_ip, 0, sizeof(host_ip));
    if (host.length() == 0)
        ret = E_tcp_invalid_host_name;
    else if (inet_aton(host.c_str(), &host_ip) != 0)
        {
//        std::cout << "get host by addr " << host << std::endl;
        if (gethostbyaddr_r((char *)&host_ip.s_addr, sizeof(host_ip.s_addr), AF_INET, &he, buf, sizeof(buf), &ptr, &error) == 0)
            {
            host_name = he.h_name;
//            std::cout << "found host name by addr " << host_name << std::endl;
            }
        else
            {
            host_name = "Unknown";
//            std::cout << "failed to find host name by addr " << host_name << std::endl;
            }
        }
    else if (gethostbyname_r(host.c_str(), &he, buf, sizeof(buf), &ptr, &error) == 0)
        {
        host_name = host;
//        std::cout << "found host name by name " << host_name << std::endl;
        memcpy(&host_ip.s_addr, *he.h_addr_list, sizeof(host_ip.s_addr));
        }
    else
        {
//        std::cout << "failed to find host name by addr " << host_name << std::endl;
        ret = E_tcp_host_not_defined;
        }
    return (ret);
    }




// private function given a port name or port number looks up this information and fills in
// the supplied arguments. It is legal to have a port number that has no name, but not the
// other way round. Note the use of reentrant function to be multi-thread safe (_r).
// Any error will be returned to the caller.
// BUG. In Linux gcc libc the getservbyname_r() and getservbyaddr_r() reentrant functions have bugs
// in them. If the service isnt found in /etc/services etc the code will SEGV when run in separate
// threads. The mutex is only there to serialise and prevent the bug. If the service is found, the code
// works as it should.

int tcp_connection::pri_set_port_info(const string &port, string &port_name, int &port_number)
    {
    int ret = E_tcp_no_error;
    struct servent se, *ptr;
    char buf[8192];

//    std::cout << "finding out about port: " << port << std::endl;
    pthread_mutex_lock(&m_lock);
    memset(&se, 0, sizeof(se));
    memset(buf, 0, sizeof(buf));
    port_name = "";
    port_number = -1;
    if (port.length() == 0)
        ret = E_tcp_invalid_service_name;
    else if (isdigit(*port.c_str()))
        {
//        std::cout << "get port by number" << std::endl;
        port_number = atoi(port.c_str());
        if (port_number <= 0 || port_number > 65535)
            ret = E_tcp_invalid_service_port;
        else if (getservbyport_r(port_number, "tcp", &se, buf, sizeof(buf), &ptr) == 0)
            {
//            std::cout << "got port name (" << se.s_name << "), len = " << strlen(se.s_name) << ", char = " << (int)se.s_name[0] << std::endl;
            port_name = se.s_name;
            }
        else
            {
//            std::cout << "didnt got port name" << std::endl;
            port_name = "Unknown";
            }
        }
    else if (getservbyname_r(port.c_str(), "tcp", &se, buf, sizeof(buf), &ptr) == 0)
        {
        port_number = se.s_port;
        port_name = se.s_name;
        }
    else
        ret = E_tcp_service_not_defined;
    pthread_mutex_unlock(&m_lock);
    return (ret);
    }

