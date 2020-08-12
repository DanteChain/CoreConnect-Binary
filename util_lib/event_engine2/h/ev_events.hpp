#pragma once

// #include <pthread.h>
// #include <semaphore.h>
// #include <errno.h>
#include <event_engine2/h/ev_error.hpp>




// forward declare classes

class ev_list;




// class defines an 'event'. Each event has a standard header part that contains a default number
// of user fields. Each field is for application use. The event may be allocated with an optional
// arbitary length buffer. The application may use this buffer for any purpose it chooses, but
// obviously must not exceed its defined size. In addition the application may derive from this
// this class to provide extra "object" functionality for a particular event.
class ev_manager;
class ev_event
    {
public:
    inline ev_event();                                              // constructor
    inline ev_event(int event);                                     // constructor
    inline ev_event(int event, int source_task_id);                 // constructor
    virtual ~ev_event(){}                                            // destructor, virtual allows for derived classes
    ev_event(const ev_event &rhs);                                  // assignment operator, don't implement
    int         m_event;                                            // event identifier must be +ve value
    int         m_src_task_id;                                      // task id of the sender
    ev_manager * pDest;                                         // pntr to next event
private:
    friend class ev_list;
    ev_event &operator=(const ev_event &);                          // assignment operator, don't implement
    ev_event    *m_next;   
    
    };





inline ev_event::ev_event() : m_event(0), m_src_task_id(-1), m_next(0)
    {
    return;
    }



inline ev_event::ev_event(int event) : m_event(event), m_src_task_id(-1), m_next(0)
    {
    return;
    }





// This class controls a list of events. Events are linked together as a classic linked
// list. There is a slight encapsulation cheat here because the next ptr is actually held
// within the event class, so to get access to it, there are friend functions in use.
// Events are usually written to the list end but exceptional events may be written to
// the list head (expedited events).


class ev_list
    {
public:
    ev_list(){}                                              // constructor
    ~ev_list(){}                                             // destructor
    inline int ev_count() const;                            // count of events on list
    void ev_flush();                                        // flush event list
    inline ev_event *ev_peep();                             // non destructive read of 1st list element
    inline ev_event *ev_read();                             // read event from list
    inline void ev_write(ev_event *pev);                    // write an event onto list
    inline void ev_write_head(ev_event *pev);               // write event to head of list
    inline void move_event_list(ev_list *src);              // move whole event list to here
private:
    ev_list(const ev_list &);                               // copy constructor, dont implement
    ev_list &operator=(const ev_list &);                    // assignment operator, dont implement
    int                 m_count;                            // no of items in list
    ev_event           *m_phead;                            // pntr to head of list
    ev_event           *m_ptail;                            // pntr to tail of list
    friend class ev_event;
    };






inline int ev_list::ev_count() const
    {
    return (m_count);
    }







inline ev_event *ev_list::ev_peep()
    {
    return (m_phead);
    }






inline ev_event *ev_list::ev_read()
    {
    ev_event *h;

    if ((h = m_phead) != 0)
        {
        --m_count;
        if ((m_phead = h->m_next) == 0)
            m_ptail = 0;
        }
    return (h);
    }




inline void ev_list::ev_write(ev_event *pev)
    {
    if (pev != 0)
        {
        pev->m_next = 0;
        if (m_phead == 0)
            m_phead = pev;
        else
            m_ptail->m_next = pev;
        m_ptail = pev;
        ++m_count;
        }
    return;
    }





inline void ev_list::ev_write_head(ev_event *pev)
    {
    if (pev != 0)
        {
        pev->m_next = m_phead;
        m_phead = pev;
        if (m_count == 0)
            m_ptail = pev;
        ++m_count;
        }
    return;
    }




// function moves the entire event list from the source specified event list to the implied
// destination list. Since this just involves adjusting a few pointers, this is very fast when moving multiple events

inline void ev_list::move_event_list(ev_list *src)
    {
    if (src == 0 || src->m_count == 0)
        ;
    else if (m_count > 0)
        {
        m_count += src->m_count;
        m_ptail->m_next = src->m_phead;
        m_ptail = src->m_ptail;
        src->m_count = 0;
        src->m_phead = 0;
        src->m_ptail = 0;
        }
    else
        {
        m_count = src->m_count;
        m_phead = src->m_phead;
        m_ptail = src->m_ptail;
        src->m_count = 0;
        src->m_phead = 0;
        src->m_ptail = 0;
        }
    return;
    }

