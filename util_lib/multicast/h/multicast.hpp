#pragma once


#include <pthread.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <logger/h/logger.hpp>



enum multicast_errors
    {
    E_mc_no_error = 0,
    E_mc_open_already = 8000,
    E_mc_socket,
    E_mc_closed_already,
    E_mc_close,
    E_mc_illegal_operation,
    E_mc_would_block,
    E_mc_read,
    E_mc_write,
    E_mc_short_write,
    E_mc_set_timestamp,
    E_mc_set_sock_reuse,
    E_mc_add_membership,
    E_mc_set_buffer,
    E_mc_bind,
    E_mc_set_write_interface,
    E_mc_set_loopback_enabled,
    E_clear_flags,
    E_get_flags,
    E_set_flags,
    };






class multicast
    {
public:
    enum open_type
        {
        mc_read,
        mc_write
        };
    enum mode
        {
        synchronous,
        asynchronous
        };
     multicast();
    ~multicast();
    int open(open_type type, const std::string &mcast_ipaddress, const std::string &port, const std::string &rxtx_interface, const int sz_buf = 65536);
    int close();
    int read(char *buf, int buf_len, int &rx_len);
    int read(char *buf, int buf_len, int &rx_len, long &kernel_usecs_timestamp, bool epoch_based = false);
    int write(const unsigned char *buf, int len);
    int set_read_timeout(long usecs);
    int fd() const;
    int mode(mode mode_type);
private:
    multicast(const multicast &);
    multicast &operator=(const multicast &);
    int p_open_for_write(int sz_buf);
    int p_open_for_read(int sz_buf);
    int p_set_flags(int fd, int flags);
    int p_clear_flags(int fd, int flags);
    std::string             m_mcast_ipaddress;
    std::string             m_port;
    std::string             m_rxtx_interface;
    int                     m_fd;
    enum mode               m_mode;
    open_type               m_open_type;
    sockaddr_in             m_dst_addr;
    };


inline long get_ktimestamp_usecs_midnight_based(struct timeval &tv)
    {
    return ((tv.tv_sec % (24L * 60L * 60L)) * 1000000L + tv.tv_usec);
    }


inline long get_ktimestamp_usecs_epoch_based(struct timeval &tv)
    {
    return (tv.tv_sec * 1000000L + tv.tv_usec);
    }

