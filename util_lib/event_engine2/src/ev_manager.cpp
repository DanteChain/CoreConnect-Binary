
#include <ev_manager.hpp>

#include <iostream>
#include <map>
#include <mutex>
#include <thread>

std::mutex g_i_mutex; // protects g_

std::mutex g_i_mutex_manfd; // protects g_

constexpr int stopEvent = 1;
constexpr int otherEvent = 2;
constexpr int startTimerEvent = 3;
constexpr int stopTimerEvent = 4;

using namespace std;

int ev_manager::ev_free(ev_event *pev)
{
    delete pev;
    return 0;
}


int ev_manager::ev_run()
{
    //run_epoll(1000);
  

    volatile bool shouldStop = false;

     m_event_loop_thread = std::thread([&,this,&shouldStop]() {
			while (!shouldStop) {
                int ret =run_epoll(1000);
			}
		});
        m_event_loop_thread.detach();

    GetQueue()->appendListener(stopEvent, [&shouldStop](ev_event *pev) {
       
        shouldStop = true;
    });
    GetQueue()->appendListener(otherEvent, [](const ev_event *pev) {
    
        if (pev->m_event == EV_ET_STARTUP)
            pev->pDest->ev_system_event_callback(pev->m_event);
        else
            pev->pDest->ev_app_event_callback(const_cast<ev_event *>(pev));
    });
    initl();
    while (!shouldStop)
    {
        usleep(1);
    }
    return 0;
}
void ev_manager::ev_enable_spinning()
{
}
int ev_manager::ev_get_task_id() const
{
    return m_task_id;
}

int ev_manager::ev_tmr_add(int tmr_id, int event, long timeout_usecs, enum ev_epoll::tmr_type type)
    {
        Timer *lcTimer = new Timer([&](Timer * tmr_called)
        {
      
                ev_event *pev = new ev_event(tmr_called->m_timer_event);
                pev->m_src_task_id = tmr_called->m_task_id;
                if (pev->m_event == 0)
                {
                    ev_tx_id(tmr_called->m_task_id, pev);
                }
                else
                {
                    ev_tx_id(tmr_called->m_task_id, pev);
                }
                if(tmr_called->m_timer_type == 0)
                {
                    
                    tmr_called->stop();
                }
            
        },timeout_usecs/1000);
        
        lcTimer->m_tmr_id = tmr_id;
        lcTimer->m_timer_event = event;
        lcTimer->m_timer_type = (int)type;

        lcTimer->m_task_id = m_task_id;
        lcTimer->start();
        m_cTimers.push_back(lcTimer);

 
        return 0;
    }

int ev_manager::ev_terminate()
    {
        for (int i = 0; i < m_cTimers.size(); i++)
            m_cTimers[i]->stop();
        GetQueue()->dispatch(stopEvent, (ev_event *)0);
        exit(0);
        return 0;
    }
std::map<int, ev_manager *> m_ev_manager_objects;
int ev_manager::ev_get_task_id_from_name(int *task_id, const char *name)
    {
    
        for (std::map<int, ev_manager *>::iterator itr = m_ev_manager_objects.begin();
             itr != m_ev_manager_objects.end(); itr++)
        {
            if (itr->second->m_name.compare(name) == 0)
            {
                *task_id = itr->second->m_task_id;
                return E_ev_no_error;
            }
        }
        return E_ev_invalid_task_name;
    }
int ev_manager::ev_tx_id(int task_id, ev_event *pev)
    {
 
        if (m_ev_manager_objects.find(task_id) != m_ev_manager_objects.end())
        {
            pev->pDest = m_ev_manager_objects[task_id];
            GetQueue()->dispatch(otherEvent, pev);
        }
       

        return 0;
    }

ev_manager::ev_manager(char const *name, int taskid)
    {
        m_name = name;
        m_task_id = m_ev_manager_objects.size();
        m_thread_id = pthread_self();
        
        
    }
bool ev_manager::initl()
    {
        m_ev_manager_objects.emplace((int)m_ev_manager_objects.size(), (ev_manager *)this);

        ev_event *e = new ev_event(EV_ET_STARTUP);


        ev_tx_id(m_task_id, e);
    }
ev_manager::~ev_manager()
    {
    }
int ev_manager::ev_alloc(ev_event **ppev)
    {
        return 0;
    }                                                        
int ev_manager::ev_build_tx_all(int event)
    {
        for (std::map<int, ev_manager *>::iterator itr = m_ev_manager_objects.begin();
             itr != m_ev_manager_objects.end(); itr++)
        {
           ev_event *pev = new ev_event(event);
            pev->m_src_task_id = itr->first;
            ev_tx_id(itr->first, pev);
        }
        return 0;
    }                                               
int ev_manager::ev_build_tx_id(int task_id, int event){
        ev_event *pev = new ev_event(event);
        pev->m_src_task_id = m_task_id;
        ev_tx_id(task_id, pev);
        return 0;
    }                                            
int ev_manager::ev_build_tx_local(int event, int position){
     for (std::map<int, ev_manager *>::iterator itr = m_ev_manager_objects.begin();
             itr != m_ev_manager_objects.end(); itr++)
        {
           ev_event *pev = new ev_event(event);
            pev->m_src_task_id = itr->first;
            ev_tx_id(itr->first, pev);
        }

        return 0;
    }                                  
int ev_manager::ev_build_tx_type(int type, int event){
        //not used
        return 0;
    }

int ev_manager::ev_fd_add(int fd, ev_epoll::event_type type, int event)
    {
        const std::lock_guard<std::mutex> lock(g_i_mutex_manfd);

        return fd_add(fd, type, event);
    }                           
void ev_manager::ev_fd_clearall()
    {
        const std::lock_guard<std::mutex> lock(g_i_mutex_manfd);
        fd_clearall();
    }
int ev_manager::ev_fd_delete(int fd, ev_epoll::event_type type, bool ignore_ctl_errors ){
    const std::lock_guard<std::mutex> lock(g_i_mutex_manfd);

    fd_delete(fd,type);
        return 0;
    }
int ev_manager::ev_fd_delete(int fd, bool ignore_ctl_errors ){
    const std::lock_guard<std::mutex> lock(g_i_mutex_manfd);

    fd_delete(fd,0);
    
    } 


void ev_manager::ev_q_flush(){}                                                                     
                                                        

int ev_manager::ev_tx_list_id(int task_id, ev_list *src_lc){
     //not used
        return 0;
    }
int ev_manager::ev_tx_local(ev_event *pev, int position){
        //not used
        return 0;
    }                              
int ev_manager::ev_tx_wait_write(int task_id, ev_event *pev)
    {
     //not used
        return 0;
    }                                 
int ev_manager::ev_get_no_active_tasks(int *pvalue){
        *pvalue = m_ev_manager_objects.size();
        return 0;
    }                                               
int ev_manager::ev_get_no_tasks(int *pvalue)
    {
        *pvalue = m_ev_manager_objects.size();
        return 0;
    }                                     


int ev_manager::ev_tmr_delete(int tmr_id)
    {
        for (int i = 0; i < m_cTimers.size(); i++)
        {
            if (m_cTimers[i]->m_tmr_id == tmr_id)
            {
                m_cTimers[i]->stop();
                m_cTimers.erase(m_cTimers.begin()+i);
            }
        }
        return 0;
    }                                                         
void ev_manager::ev_tmr_clearall()
    {
        for (int i = 0; i < m_cTimers.size(); i++)
        {
        m_cTimers[i]->stop();
        delete m_cTimers[i];
        }
        m_cTimers.clear();
            
    }                                           

void ev_manager::ev_disable_spinning(){}

int ev_manager::read_event(int fd, int eevent)
    {
        ev_event *pev = new ev_event(eevent);
        pev->m_src_task_id = m_task_id;
        ev_tx_id(m_task_id, pev);
        return E_ev_no_error;
    }
int ev_manager::write_event(int fd, int event)
    {
        return E_ev_no_error;
    }
int ev_manager::exception_event(int fd, int event)
    {
        ev_event *pev = new ev_event(event);
        pev->m_src_task_id = m_task_id;
        ev_tx_id(m_task_id, pev);
        return E_ev_no_error;
    }
int ev_manager::timer_event(int tmr_id, int event, long expired_count)
    {

        return E_ev_no_error;
    }
