#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <h/multicast.hpp>


using namespace std;




multicast::multicast() : m_fd(-1), m_mode(synchronous), m_open_type(mc_read)
    {
    memset(&m_dst_addr, 0, sizeof(m_dst_addr));
    return;
    }





multicast::~multicast()
    {
    if (m_fd != -1)
        close();
    return;
    }






int multicast::open(open_type type, const std::string &mcast_ipaddress, const std::string &port, const std::string &rxtx_interface, const int sz_buf)
    {
    int ret;

    if (m_fd != -1)
        ret = E_mc_open_already;
    else if ((m_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        ret = E_mc_socket;
    else
        {
        m_mcast_ipaddress = mcast_ipaddress;
        m_port = port;
        m_rxtx_interface = rxtx_interface;
        ret = (type == mc_read) ? p_open_for_read(sz_buf) : p_open_for_write(sz_buf);
        if (ret != E_mc_no_error)
            close();
        }
    return (ret);
    }





int multicast::close()
    {
    int ret;

    if (m_fd == -1)
        ret = E_mc_closed_already;
    else if (::close(m_fd) == -1)
        ret = E_mc_close;
    else
        {
        m_fd = -1;
        ret = E_mc_no_error;
        }
    return (ret);
    }





int multicast::read(char *buf, int buf_len, int &rx_len)
    {
    int ret;
    int n;

    rx_len = 0;
    if (m_fd == -1)
        ret = E_mc_closed_already;
    else if (m_open_type == mc_write)
        ret = E_mc_illegal_operation;
    else if ((n = recvfrom(m_fd, buf, buf_len, 0, NULL, NULL)) >= 0)
        {
        rx_len = n;
        ret = E_mc_no_error;
        }
    else if (errno == EINTR)
        ret = E_mc_no_error;
    else if (errno == EAGAIN)
        ret = E_mc_would_block;
    else
        ret = E_mc_read;
    return (ret);
    }




int multicast::read(char *buf, int buf_len, int &rx_len, long &kernel_usecs_timestamp, bool epoch_based)
    {
    char ctrl_buf[2048];
    struct msghdr msg;
    struct cmsghdr *cmsg = (struct cmsghdr *)ctrl_buf;
    struct iovec iov;
    struct timeval time_kernel;
    int ret;

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = ctrl_buf;
    msg.msg_controllen = sizeof(ctrl_buf);
    iov.iov_base = buf;
    iov.iov_len = buf_len;
    rx_len = 0;
    if (m_fd == -1)
        ret = E_mc_closed_already;
    else if (m_open_type == mc_write)
        ret = E_mc_illegal_operation;
    else if ((rx_len = recvmsg(m_fd, &msg, 0)) > 0)
        {
        kernel_usecs_timestamp = 0;
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMP && cmsg->cmsg_len == CMSG_LEN(sizeof(time_kernel)))
            kernel_usecs_timestamp = (epoch_based == true) ? get_ktimestamp_usecs_epoch_based(*(struct timeval *)CMSG_DATA(cmsg)) : get_ktimestamp_usecs_midnight_based(*(struct timeval *)CMSG_DATA(cmsg));
        ret = E_mc_no_error;
        }
    else if (errno == EINTR)
        ret = E_mc_no_error;
    else if (errno == EAGAIN)
        ret = E_mc_would_block;
    else
        ret = E_mc_read;
    return (ret);
    }







int multicast::set_read_timeout(long usecs)
    {
    int ret = E_mc_no_error;
    struct timeval tv;

    if (m_fd == -1)
        ret = E_mc_closed_already;
    else if (m_open_type == mc_write)
        ret = E_mc_illegal_operation;
    else
        {
        tv.tv_sec = usecs / 1000000;
        tv.tv_usec = usecs % 1000000;
        fflush(stdout);
        if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) != 0)
            ret = E_mc_illegal_operation;
        }
    return (ret);
    }






int multicast::write(const unsigned char *buf, int len)
    {
    int ret;
    int n;

    if (m_fd == -1)
        ret = E_mc_closed_already;
    else if (m_open_type == mc_read)
        ret = E_mc_illegal_operation;
    else if ((n = sendto(m_fd, buf, len, 0, (struct sockaddr *)&m_dst_addr, sizeof(m_dst_addr))) == -1)
        {
        if (errno == EAGAIN)
            ret = E_mc_would_block;
        else
            ret = E_mc_write;
        }
    else if (n != len)
        ret = E_mc_short_write;
    else
        ret = E_mc_no_error;
    return (ret);
    }






int multicast::fd() const
    {
    return (m_fd);
    }





int multicast::mode(enum mode mode_type)
    {
    int ret;

    if (m_fd == -1)
        ret = E_mc_closed_already;
    else if (mode_type == asynchronous)
        ret = p_set_flags(m_fd, O_NDELAY);
    else
        ret = p_clear_flags(m_fd, O_NDELAY);
    if (ret == E_mc_no_error)
        m_mode = mode_type;
    return (ret);
    }





int multicast::p_open_for_read(int sz_buf)
    {
    int val1 = 1, enabled = 1; //, val2 = 4, num = 0;
    int ret;
    struct ip_mreq mc;
    sockaddr_in src_addr;

    m_open_type = mc_read;
    mc.imr_multiaddr.s_addr = inet_addr(m_mcast_ipaddress.c_str());
    mc.imr_interface.s_addr = inet_addr(m_rxtx_interface.c_str());
    if (setsockopt(m_fd, SOL_SOCKET, SO_TIMESTAMP, (char *)&enabled, sizeof(enabled)) != 0)
        ret = E_mc_set_timestamp;
    else if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&val1, sizeof(val1)) != 0)
        ret = E_mc_set_sock_reuse;
    else if (setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mc, sizeof(mc)) == -1)
        ret = E_mc_add_membership;
    else
        {
        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.sin_family = AF_INET;
        src_addr.sin_addr.s_addr = inet_addr(m_mcast_ipaddress.c_str());
        src_addr.sin_port = htons(atoi(m_port.c_str()));
        if (setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&sz_buf, sizeof(sz_buf)) != 0)
            ret = E_mc_set_buffer;
        else if (bind(m_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) == -1)
            ret = E_mc_bind;
//        else if (getsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&num, (socklen_t *)&val2) != 0)          MDW for testing only
//            ret = E_mc_set_buffer;
        else
            {
            ret = E_mc_no_error;
//            std::cout << "rx buf = " << num << std::endl;
            }
        }
    return (ret);
    }






#if 0
int multicast::p_open_for_read(int sz_buf)
    {
    int val1 = 1, val2 = 4, num = 0;
    int ret;
    struct ip_mreq mc;
    sockaddr_in src_addr;

    m_open_type = mc_read;
    mc.imr_multiaddr.s_addr = inet_addr(m_mcast_ipaddress.c_str());
    mc.imr_interface.s_addr = inet_addr(m_rxtx_interface.c_str());
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&val1, sizeof(val1)) != 0)
        ret = E_mc_set_sock_reuse;
    else if (setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mc, sizeof(mc)) == -1)
        ret = E_mc_add_membership;
    else
        {
        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.sin_family = AF_INET;
        src_addr.sin_addr.s_addr = inet_addr(m_mcast_ipaddress.c_str());
        src_addr.sin_port = htons(atoi(m_port.c_str()));
        if (bind(m_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) == -1)
            ret = E_mc_bind;
        else if (setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&sz_buf, sizeof(sz_buf)) != 0)
            ret = E_mc_set_buffer;
//        else if (getsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&num, (socklen_t *)&val2) != 0)          MDW for testing only
//            ret = E_mc_set_buffer;
        else
            {
            ret = E_mc_no_error;
//            std::cout << "rx buf = " << num << std::endl;
            }
        }
    return (ret);
    }
#endif




int multicast::p_open_for_write(int sz_buf)
    {
    int ret;
    in_addr x;
    char flag = 1;

    m_open_type = mc_write;
    x.s_addr = inet_addr(m_rxtx_interface.c_str());
    if (setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_IF, &x, sizeof(x)) == -1)
        ret = E_mc_set_write_interface;
    else if (setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag)) == -1)
        ret = E_mc_set_loopback_enabled;
    else if (setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (char *)&sz_buf, sizeof(sz_buf)) != 0)
        ret = E_mc_set_buffer;
    else
        {
        memset(&m_dst_addr, 0, sizeof(m_dst_addr));
        m_dst_addr.sin_family = AF_INET;
        m_dst_addr.sin_port = htons(atoi(m_port.c_str()));
        m_dst_addr.sin_addr.s_addr = inet_addr(m_mcast_ipaddress.c_str());
        ret = E_mc_no_error;
        }
    return (ret);
    }






int multicast::p_clear_flags(int fd, int flags)
    {
    int val;
    int ret;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        ret = E_get_flags;
    else if (val &= ~flags, fcntl(fd, F_SETFL, val) < 0)
        ret = E_clear_flags;
    else
        ret = E_mc_no_error;
    return (ret);
    }





int multicast::p_set_flags(int fd, int flags)
    {
    int ret;
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        ret = E_get_flags;
    else if (val |= flags, fcntl(fd, F_SETFL, val) < 0)
        ret = E_set_flags;
    else
        ret = E_mc_no_error;
    return (ret);
    }




