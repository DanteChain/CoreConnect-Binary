#include <ev_epoll.hpp>
#include <iostream>
#include <mutex>

std::mutex g_i_mutex_fd; // protects g_

ev_epoll::ev_epoll()
    {
    m_event_poll_t = event_init();
    if(!m_event_poll_t)
    cout << " event initialization failed" << endl;
    }
ev_epoll::~ev_epoll()
    {
        
    }
int event_poll_callback_impl(int fd, short  int type, void * arg)
    {
    const std::lock_guard<std::mutex> lock(g_i_mutex_fd);

    int ret(0);

    fd_data * fd_datap = ((fd_data *)arg);
    if ((type&E_POLLIN  ||type&E_POLLPRI))
        fd_datap->source->read_event(fd, fd_datap->event);
    if ((type&E_POLLOUT))
        fd_datap->source->write_event(fd, fd_datap->event);

    else if (type&E_POLLERR)
    {
        fd_datap->source->exception_event(fd, fd_datap->event);
        ret = -1;
    }
    else  if (type&E_POLLHUP)
    {
        fd_datap->source->exception_event(fd, fd_datap->event);

        ret = -1;
    }
    else  if (type&E_POLLNVAL) {
        fd_datap->source->exception_event(fd, fd_datap->event);

        ret = -1;
    }

    return ret;
    }
int event_timer_callback_impl(void * args)
    {
    return 0;

    }
struct key
    {
    short int fd;
    short int epollfd;

    };
int ev_epoll::fd_add(int fd, enum event_type type, int event)
    {
    const std::lock_guard<std::mutex> lock(g_i_mutex_fd);

    short int checkevents(0);
    int epollfd(0);
    fd_data * fd_new = new fd_data{ fd, event, type, epollfd, this };

    if (type == ev_fd_type_read)
        checkevents = E_POLLIN;
    if (type == ev_fd_type_write)
        checkevents = E_POLLOUT;
    if (type == ev_fd_type_except)
        checkevents = E_POLLERR|E_POLLHUP|E_POLLNVAL;


    fd_new->epollfd = event_add_fd(m_event_poll_t, fd, checkevents, fd_new, event_poll_callback_impl);
    key k;
    k.fd = fd;
    k.epollfd = fd_new->epollfd;
    int mapKey;
    memcpy(&mapKey, &k, 4);
    vecfd.insert(std::make_pair(mapKey, fd_new));

    return 0;
    }
int ev_epoll::fd_delete(int fd, enum event_type type, bool ignore_ctl_errors)
    {
    const std::lock_guard<std::mutex> lock(g_i_mutex_fd);

    int ret(0);


    for (std::map<int, fd_data*>::iterator iter = vecfd.begin();
        iter != vecfd.end();
        iter++
        )

    {
        if (iter->second->fd ==fd) {
            ret =event_remove_fd(m_event_poll_t, iter->second->epollfd);

            delete iter->second;
            vecfd.erase(iter);
        }
    }
    return ret;
    }
int ev_epoll::fd_delete(int fd, bool ignore_ctl_errors)
    {
    const std::lock_guard<std::mutex> lock(g_i_mutex_fd);

    int ret(0);
    for (std::map<int, fd_data*>::iterator iter = vecfd.begin();
        iter != vecfd.end();
        iter++
        ) {
        if (iter->second->fd ==fd) {
            ret = event_remove_fd(m_event_poll_t, iter->second->epollfd);

            delete iter->second;
            vecfd.erase(iter);
        }
    }
    return ret;
    }
void ev_epoll::fd_clearall()
    {

    const std::lock_guard<std::mutex> lock(g_i_mutex_fd);

    for (std::map<int, fd_data*>::iterator iter =vecfd.begin();
        iter !=vecfd.end();
        iter++
        )
    {
        int ret = event_remove_fd(m_event_poll_t, iter->second->epollfd);

        delete iter->second;
    }
    vecfd.clear();

    }   
int ev_epoll::tmr_add(int tmr_id, enum tmr_type, int event, long timeout_usecs) 
    {

    event_add_timer(m_event_poll_t, timeout_usecs, &event, event_timer_callback_impl);
    return 0;
    }
int ev_epoll::tmr_delete(int tmr_id) 
    {
    return 0;
    }
    void ev_epoll::tmr_clearall(){

    }
    int ev_epoll::run_epoll(int timeout_msecs)
    {
        event_main_loop(m_event_poll_t);
    return 0;
    }
    void ev_epoll::reset_stats(){}
    void ev_epoll::get_stats(long &epoll_count, long &eintr_count, bool reset_statistics){}
    void ev_epoll::debug_dump(){}
    int ev_epoll::set_fd_non_blocking(int fd)
    {
    return 0;
    }
    int ev_epoll::set_fd_blocking(int fd)
    {
    return 0;
    }
