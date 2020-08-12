#pragma once

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/eventfd.h>

//#include <iostream>
#include <chrono>
#include <functional>
#include <thread>


#include <event_engine2/h/ev_events.hpp>
#include <event_engine2/h/ev_epoll.hpp>
#include <event_engine2/h/ev_error.hpp>
#include <time/h/timestamp.hpp>
#include <eventqueue.h>
#include <eventpolicies.h>
#include <vector>


#define EV_USE_SPIN_LOCKS                   (1)             // make code tidier when switching between mutex and spin locks

#if EV_USE_SPIN_LOCKS == 1
    #define TASK_LOCK(X)                        pthread_spin_lock(&ta[X].spin_lock)
    #define TASK_UNLOCK(X)                      pthread_spin_unlock(&ta[X].spin_lock)
#else
    #define TASK_LOCK(X)                        pthread_mutex_lock(&ta[X].task_lock)
    #define TASK_UNLOCK(X)                      pthread_mutex_unlock(&ta[X].task_lock)
#endif







#define EV_MAX_NO_TASKS             (512)                   // arb max number of tasks allowed
#define EV_EPOLL_INFINITE_TIMEOUT   (-1)                    // wait on epoll forever
#define EV_EPOLL_NO_WAIT_TIMEOUT    (0)                     // return from epoll immediately, used for spinning




// Defines which end of an event list the event is written to. Writing to the head is usually
// only for expidated data.

#define EV_WRITE_HEAD               (0)                     // write to head of event q
#define EV_WRITE_TAIL               (1)                     // write to tail of event q



// Define all the internal system events that may be generated automatically and that an
// application must expect. The startup event is always the first event an application
// receives, it is there to allow the app to gain control and start things off. The q input
// event is internal and will never be seen by the application. The exception event will
// be generated if an exception has occured on an fd and the application has not registered to
// process it. Exception events MUST be allowed for in an application since they cannot be
// masked in poll().

#define EV_ET_FD_EXCEPTION          (-3)                    // an fd suffered an exception event that wasnt watched
#define EV_ET_Q_INPUT               (-2)                    // internal event queueing event
#define EV_ET_STARTUP               (-1)                    // first event given to a new task





// Defines the states the event manager may be in.

#define EV_STATE_ACTIVE             (0)                     // task active and running
#define EV_STATE_TERMINATED         (1)                     // task marked for death or is already dead






class ev_manager;                                           // forward declare usage of ev_manager class




// defines an array of pointers to ev_manager objects and a lock for each one. This array
// is used for all inter task message communications. All messaging between tasks must
// utilise the mutex lock otherwise undefined results may occur.

typedef struct _ev_task_array
     {
#if EV_USE_SPIN_LOCKS == 1
     pthread_spinlock_t     spin_lock;
#else
     pthread_mutex_t        task_lock;                      // task shared data lock
#endif
     ev_manager             *ptask;                         // ptr to task header
     } EV_TASK_ARRAY;




// This class is in overall control of the event manager system. Once it is created and control handed
// to it, the application will only regain control once an event is ready to process, irrespective of the
// source of the event. This encourages the application to be written in the style of a Finite State
// Machine (FSM). FSM's are very good for describing communication protocols and event driven software.
// This class is by no means complete and can be refined, but as usual time constraints limit the
// work that can be done initially.
/*
    COPYRIGHT © 2018 Ringo Hoffmann (zekro Development)
    READ BEFORE USING: https://zekro.de/policy
*/

#pragma once



/**
 *  Create asynchronous timers which execute specified
 *  functions in set time interval.
 *  
 *  @param func		Function which sould be executed
 *  @param interval	Interval of time in which function will be executed
 *					(in milliseconds)
 */
class Timer {

public:
int m_tmr_id;
int m_task_id;
int m_timer_event;
int m_timer_type;
	Timer() {}

	Timer(std::function<void(Timer*)> func, const long &interval) {
		m_func = func;
		m_interval = interval;
	}

	/**
	 * Starting the timer.
	 */
	void start() {
		m_running = true;
		m_thread = std::thread([&]() {
			while (m_running) {
				auto delta = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_interval);
				m_func(this);
				std::this_thread::sleep_until(delta);
			}
		});
		m_thread.detach();
	}

	/*
	 *  Stopping the timer and destroys the thread.
	 */
	void stop() {
		m_running = false;
		m_thread.~thread();
	}

	/*
	 *  Restarts the timer. Needed if you set a new
	 *  timer interval for example.
	 */
	void restart() {
		stop();
		start();
	}
	
	/*
	 *  Check if timer is running.
	 *  
	 *  @returns boolean is running
 	 */
	bool isRunning() { 
		return m_running; 
	}


	/*
	*  Set the method of the timer after
	*  initializing the timer instance.
	*
	*  @returns boolean is running
	*  @return  Timer reference of this
	*/
	Timer *setFunc(std::function<void(Timer*)> func) {
		m_func = func;
		return this;
	}

	/*
	 *  Returns the current set interval in milliseconds.
	 *
	 *  @returns long interval
	 */
	long getInterval() { 
		return m_interval; 
	}

	/*
	*  Set a new interval for the timer in milliseconds.
	*  This change will be valid only after restarting
	*  the timer.
	*
	*  @param interval new interval
	*  @return  Timer reference of this
	*/
	Timer *setInterval(const long &interval) {
		m_interval = interval;
		return this;
	}

	~Timer() {
		stop();
	}

private:
	// Function to be executed fater interval
	std::function<void(Timer*)> m_func;
	// Timer interval in milliseconds
	long m_interval;

	// Thread timer is running into
	std::thread m_thread;
	// Status if timer is running
	bool m_running = false;
};
struct MyEventPolicies {
	using Threading = eventpp::MultipleThreading;
};
typedef eventpp::EventDispatcher<int, void (ev_event*),MyEventPolicies> EPPQ;

class ev_manager : public ev_epoll
     {
public:
 std::vector<Timer*> m_cTimers;
 bool initl();
typedef  eventpp::EventQueue<int, void (ev_event*)> EPPQ;
EPPQ queue;
    EPPQ *GetQueue(){return &queue;}
    ev_manager(const char *name, int type);
   
    virtual ~ev_manager();                                                                  // destructor
    int ev_alloc(ev_event **ppev);                                                          // allocate an event
    int ev_build_tx_all(int event);                                                         // build tx event to all tasks
    int ev_build_tx_id(int task_id, int event);                                             // build tx event to task id
    int ev_build_tx_local(int event, int position);                                         // build tx event to local q
    int ev_build_tx_type(int type, int event);                                              // build tx event to all tasks of a type
    int ev_fd_add(int fd, ev_epoll::event_type type, int event);                            // add an fd to be watched
    void ev_fd_clearall();                                                                  // clear all watched fd's
    int ev_fd_delete(int fd, ev_epoll::event_type type, bool ignore_ctl_errors = false);    // delete a watched fd
    int ev_fd_delete(int fd, bool ignore_ctl_errors = false);                               // delete a watched fd all types of watching
    int ev_free(ev_event *pev);                                                             // free an event + optional buffer
    int ev_q_count() const;                                                                 // get count of events on q
    void ev_q_flush();                                                                      // clear the waiting event queue
    int ev_tx(ev_event *pev);                                                               // write event to external event list
    int ev_tx_id(int task_id, ev_event *pev);                                               // tx event to task id
    int ev_tx_list_id(int task_id, ev_list *src_lc);                                        // tx event list to id
    int ev_tx_local(ev_event *pev, int position);                                           // tx event to the local queue (very fast)
    int ev_tx_wait_write(int task_id, ev_event *pev);                                       // tx event to external event list and wait
    int ev_get_no_active_tasks(int *pvalue);                                                // get current no active tasks running
    int ev_get_no_tasks(int *pvalue);                                                       // get current no of tasks running
    int ev_get_task_id() const;                                                             // get own task id
    int ev_get_task_id_from_name(int *task_id, const char *name);                           // get a task id from a given name
    int ev_get_task_type() const;                                                           // get type of task
     int ev_run();                                                                           // run the event manager
    int ev_terminate();                                                                     // terminate the event manager
    int ev_tmr_add(int tmr_id, int event, long timeout_usecs, enum ev_epoll::tmr_type type);
    int ev_tmr_delete(int tmr_id);                                                          // stop a real-time timer
    void ev_tmr_clearall();                                                                 // stop all timers
    void ev_enable_spinning();
    void ev_disable_spinning();
    virtual int ev_app_event_callback(ev_event *pev) = 0;
    virtual int ev_system_event_callback(int event) = 0;
          int                     m_task_id;                                                      // my task id
private:
   //  int ev_tx_tmr(int task_id, ev_event *pev);
    // ev_manager(const ev_manager &);                                                         // copy constructor, don't implement
    // ev_manager &operator=(const ev_manager &);                                              // assignment operator, don't implement
    virtual int read_event(int fd, int event);
    virtual int write_event(int fd, int event);
    virtual int exception_event(int fd, int event);
    virtual int timer_event(int tmr_id, int event, long expired_count);
    // int p_alloc_task_slot();                                                                // allocate an empty task slot in array
    // int p_build_tx_local(int event, int position);                                          // build and q event to process
    // int p_event_write(int task_id, ev_event *pev);                                          // write event to event list
    // int p_ev_build_tx_all(int event);                                                       // build tx event to all tasks
    // int p_ev_build_tx_id(bool external, int task_id, int event);                            // build tx event to task id
    // int p_ev_build_tx_type(int type, int event);                                            // build tx event to task types
    // int p_ev_tx_id(int task_id, ev_event *pev);                                             // tx event to task id
    // int p_ev_tx_list_id(int task_id, ev_list *src_lc);                                      // tx event list to task id
    // int p_free_task_slot();                                                                 // free task slot up
    // int p_get_task_id_from_name(int *task_id, const char *name);                            // get a task id froma task name
    inline int p_read_event_fd();
    inline int p_write_event_fd();
    inline int p_read_input_q();                                                            // read FIFO, move external events to local queue
    inline void p_read_input_q_spinning();
     int p_remove();                                                                         // remove all events etc from event manager
     void p_set_mgr_state(int state);                                                        // sets the event manager state
     ev_list                m_rel[2];                                                           // remote event list
     ev_list                m_lel;                                                           // local event list
     ev_list                 *m_relp;
     bool                    m_x;
     string                  m_name;
     bool                    m_event_list_busy;                                              // is external event list busy
     int                     m_event_fd;
     //int                     m_state;                                                        // state of task
     int                     m_task_type;                                                    // type of task
     pthread_t               m_thread_id;                                                    // my thread id
     std::thread m_event_loop_thread;
     int                     m_epoll_timeout;
     bool                    m_spinning;
     static pthread_mutex_t  master_lock;                                                    // tasking system master lock
     static int              no_tasks;                                                       // number active tasks
     static int              no_active_tasks;                                                // tasks currently active
     static EV_TASK_ARRAY    ta[EV_MAX_NO_TASKS];                                            // task array, pntrs and locks
    };




inline int ev_manager::p_read_event_fd()
    {
    long val;

    if (read(m_event_fd, &val, sizeof(val)) == sizeof(val))
        return (E_ev_no_error);
    return (E_ev_clear_failed);
    }





inline int ev_manager::p_write_event_fd()
    {
    long val = 1;

    if (write(m_event_fd, &val, sizeof(val)) == sizeof(val))
        return (E_ev_no_error);
    return (E_ev_poke_failed);
    }






// // internal function reads the FIFO queue, then moves any external events from the remote
// // event list and appends them to the local event list.

inline int ev_manager::p_read_input_q()
    {
    int ret;
    ev_list *tmp = m_relp;

    m_x ^= 1;
    if ((ret = p_read_event_fd()) == E_ev_no_error)
        {
        TASK_LOCK(m_task_id);
        m_event_list_busy = false;
        m_relp = &m_rel[m_x];
        TASK_UNLOCK(m_task_id);
        m_lel.move_event_list(tmp);
        }
    return (ret);
    }




// inline void ev_manager::p_read_input_q_spinning()
//     {
//     ev_list *tmp = m_relp;

//     m_x ^= 1;
//     TASK_LOCK(m_task_id);
//     m_relp = &m_rel[m_x];
//     TASK_UNLOCK(m_task_id);
//     if (tmp->ev_count() > 0)
//         m_lel.move_event_list(tmp);
//     return;
//     }

