#pragma once

#include <pthread.h>
#include <cstring>
#include <climits>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <event_engine2/h/ev_error.hpp>
#include <list>
#include <map>
extern "C"{
#include "event_poll.h"
}
using namespace std;


class ev_epoll;

 typedef struct _fd_data
{
    int fd;
    int event;
    int type;
    int epollfd;
    ev_epoll * source;
} fd_data ;
class ev_epoll
    {
public:
    ev_epoll();
    virtual ~ev_epoll();
    std::map<int,fd_data*> vecfd;
    enum tmr_type
        {
        ev_tmr_type_oneshot = 0,
        ev_tmr_type_continuous = 1,
        ev_tmr_type_unknown = 2,
        };
    enum event_type
        {
        ev_fd_type_read = 0,
        ev_fd_type_write = 1,
        ev_fd_type_except = 2,
        ev_no_fd_types = 3,
        };
    int fd_add(int fd, enum event_type type, int event);
    int fd_delete(int fd, enum event_type type, bool ignore_ctl_errors = false);
    int fd_delete(int fd, bool ignore_ctl_errors = false);
    void fd_clearall();
    int tmr_add(int tmr_id, enum tmr_type, int event, long timeout_usecs);
    int tmr_delete(int tmr_id);
    void tmr_clearall();
    int run_epoll(int timeout_msecs);
    void reset_stats();
    void get_stats(long &epoll_count, long &eintr_count, bool reset_statistics = false);
    void debug_dump();
    int set_fd_non_blocking(int fd);
    int set_fd_blocking(int fd);
    virtual int read_event(int fd, int event) = 0;
    virtual int write_event(int fd, int event) = 0;
    virtual int exception_event(int fd, int event) = 0;
    virtual int timer_event(int tmr_id, int event, long expired_count) = 0;
private:

    event_poll_t *m_event_poll_t;
    ev_epoll(const ev_epoll &);
    ev_epoll &operator=(const ev_epoll &);
    enum event_class
        {
        class_fd = 0,
        class_tmr = 1,
        };
    typedef struct
        {
        int                     fd;
        int                     tmr_id;
        struct epoll_event      ep_ev;
        enum event_class        ev_class;
        enum tmr_type           tmr_types;
        int                     user_events[ev_no_fd_types];
        } EVENT_INFO;
    static const int            mc_max_events_to_process = 128;
    static const unsigned int   mc_max_no_fds = 1024;
    static const int            mc_max_fd_value = 65536;
    static const int            mc_max_tmr_value = 65536;
    static const unsigned int   mc_max_no_tmrs = 10;
    static const int            mc_min_event_value = -50;
    static const int            mc_max_event_value = INT_MAX;
    static const long           mc_max_tmr_timeout = (1000000L * 86400L * 100L);             // 100 days max timeout in usecs
    static const int            mc_epoll_type[ev_no_fd_types];
    EVENT_INFO *p_fd_find(int fd);
    int p_fd_add(int fd, enum event_type type, int event);
    int p_fd_delete(int fd, enum event_type type, bool ignore_ctl_errors = false);
    int p_fd_delete(int fd, bool ignore_ctl_errors = false);
    void p_fd_clearall();
    EVENT_INFO *p_tmr_find(int tmr_id);
    int p_tmr_add(int tmr_id, enum tmr_type type, int event, long timeout);
    int p_tmr_delete(int tmr_id);
    int p_tmr_delete(EVENT_INFO *p);
    void p_tmr_clearall();
    void p_dead_clearall();
    void p_event_info_attempt_delete(EVENT_INFO *p);
    int                         m_epoll_fd;
    list<EVENT_INFO *>          m_fds;
    list<EVENT_INFO *>          m_dead;
    list<EVENT_INFO *>          m_tmrs;
    long                        m_epoll_count;
    long                        m_eintr_count;
    struct epoll_event          m_events[mc_max_events_to_process];
    bool                        m_in_run_epoll;
    };

