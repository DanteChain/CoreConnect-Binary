#pragma once

#include <pthread.h>
#include <string>
#include <iostream>
#include <strstream>
#include <deque>




// class defines functionality required to support logging to a specified stream and/or file
// from within a multi-thread program. It is designed to be simple, fast, and thread safe. The
// buffer size is arbitary and may be changed. The class may be used as is, or more usually
// the logging is accessed via the ES_LOG macro that performs the thread safe locking on the
// users behalf.
// NOTE NOTE NOTE The log buffer for the data has an extra 200 added to it to make sure there is
// enough room always for a date/timestamp + name + log level string. Yes I know 200 is a frig,
// but it is fine for now, just be aware of it. The line header must never exceed this size or
// memory corruption may occur and will be bloody difficult to find. One way to ensure it would
// be to make sure the name passed to the macro is always < (200 - timestamp - level len) or simpler
// in that the name is always <= 128 bytes. This is not checked at the moment, but it could be.
// NOTE NOTE NOTE unbuffered logging interacts badly with the async write mode as each log line
// has to be sent to the disk writer thread for writing which is quite slow and a very large number
// of pool buffers can be allocated in this mode.
// Recommended modes are buffered & async or unbuffered & sync

class logger : public std::strstream
    {
public:
    logger(std::ostream &out_stream, const char *file_name = 0);// Constructor
    virtual ~logger();                                          // Destructor
    enum log_level                                              // logging debug levels allowed
        {                                                       // each level type will be printed
        Debug_9,                                                // out at the start of every logged
        Debug_8,                                                // out at the start of every logged
        Debug_7,                                                // out at the start of every logged
        Debug_6,                                                // out at the start of every logged
        Debug_5,                                                // out at the start of every logged
        Debug_4,                                                // out at the start of every logged
        Debug_3,                                                // out at the start of every logged
        Debug_2,                                                // out at the start of every logged
        Debug_1,                                                // out at the start of every logged
        Debug_0,                                                // out at the start of every logged
        Info,                                                   // message. Anything logged with at the
        Warning,                                                // always level will always be logged
        Error,                                                  //
        Fatal,
        Always
        };
    void set_new_file(const char *file_name);                           // change the filename of the log file
    void disable_buffered();                                            // disable the buffering logic
    void disable_file();                                                // stop all output to specified file
    void disable_stream();                                              // stop all output to specified stream
    void disable_async();                                               // disable the asynchronous write logic
    void enable_async();                                                // enable the asynchronous write logic
    void enable_buffered();                                             // enable the buffering logic
    void enable_file();                                                 // enable all output to specifed file
    void enable_stream();                                               // enable all ouptut to specified stream
    void flush();                                                       // explicit flush of any data to disk/stream
    enum log_level get_level();                                         // get the current logging level
    enum log_level get_force_flush_level();                             // get the current force flush logging level
    const char *get_str_level();
    bool get_level_from_str(enum log_level &level, const char *str);    // get the level from a C string representation
    const char *get_str_level(enum log_level level);                    // get logging level as a C string
    int get_str_level_len(enum log_level level);                        // get logging level C string length
    logger &lock();                                                     // mutex lock support
    bool logging_required(enum log_level level) const;                  // ask if line at right level to be logged
    void process_data(const std::string &name, enum log_level level);   // process stream data
    enum log_level set_level(enum log_level new_level);                 // set the new logging level
    enum log_level set_level(const char *ll_str);                       // set new logging level via a C string matching level names
    enum log_level set_force_flush_level(enum log_level new_level);     // set the new force a flush log level
    const char *timestamp();                                            // get curent timestamp
    logger &unlock();                                                   // mutex unlock support
private:
    enum                                                                // reasonable length for log buffer. With async enabled the logger will
        {                                                               // have at least 2 of these created as the buffers pass back and forwards
        sz_log_buf = 1024 * 1024 * 2,                                   // between the disk writing thread.
        sz_log_buf_extra = 200                                          // Extra space frig for log header data/time/level etc. 200 is easily enough.
        };
    struct pool_entry
        {
        char            *buf;                                           // ptr to log buffer
        int             len;                                            // current used length of fixed size log buffer
        };
    logger(const logger &);                                             // copy constructor, DO NOT IMPLEMENT
    logger &operator=(const logger &);                                  // assignment operator, DO NOT IMPLEMENT
    static const char *str_log_levels[];                                // static array of string names for debug levels
    static const size_t str_log_levels_len[];                           // static array of string name lengths for debug levels
    static const enum log_level enum_log_levels[];                      // static array of valid log levels
    static const int    m_timestamp_len = 25;                           // len in bytes of current timestamp
    void p_do_work();                                                   // do the work of writing the data out
    void send_to_files(const char *buf, int len);                       // write buffer to disk
    void send_to_files(std::deque<struct pool_entry> &waiting_pool);    // function called from disk thread to write multiple buffers at once
    void disk_thread();                                                 // disk writer thread
    static void *thread_thunk(void *);                                  // function called to create disk thread
    pthread_mutex_t     m_lock;                                         // lock for inter thread synchronisation
    pthread_mutex_t     m_io_lock;                                      // io lock used by send_to_files (this lock MUST always be taken last to avoid deadlocks)
    pthread_cond_t      m_write_cv;                                     // condition variable - is data ready for writing?
    pthread_mutex_t     m_write_mutex;                                  // mutex to support m_write_cv
    pthread_t           m_write_tid;                                    // writer's thread tid
    std::deque<struct pool_entry> m_dirty_pool;                         // pool of buffers that need to be flushed to disk
    std::deque<struct pool_entry> m_clean_pool;                         // pool of buffers that have been flushed to disk and can be re-used
    bool                m_terminate;                                    // is logger shutting down?
    char                *m_buf;                                         // output log buffer
    char                *m_cptr;                                        // current output log ptr
    char                *m_file_name;                                   // optional filename to write to
    std::ostream        &m_out_stream;                                  // optional stream to write to
    bool                m_buffered;                                     // is data to be buffered or not
    bool                m_async;                                        // is data to be written to disk asynchronously (in another write thread)
    bool                m_stream_out;                                   // is stream output enabled or not
    bool                m_file_out;                                     // is file output enabled or not
    char                m_timestamp[30];                                // current timestamp YYYYMMDD HH:MM:SS.MMMMMM " 25 bytes
    time_t              m_time;                                         // saved system time in secs
    time_t              m_usecs;                                        // saved system time in microsecs
    enum log_level      m_log_level;                                    // current logging level
    enum log_level      m_force_flush_log_level;                        // current force a flush log level
    };







// function returns a pointer to the C string representation of the current
// logging level
// This does not need locking since it accesses read only data.

inline const char *logger::get_str_level(enum log_level level)
    {
    return (str_log_levels[(int)level]);
    }



inline int logger::get_str_level_len(enum log_level level)
    {
    return (str_log_levels_len[(int)level]);
    }



// function locks the object so that operations on it are thread safe.

inline logger &logger::lock()
    {
    pthread_mutex_lock(&m_lock);
    return (*this);
    }




// function returns a boolean value indicating whether logging of the spocified
// level is required or not
// NOTE
// this must only be called between nested lock() / unlock() functions.

inline bool logger::logging_required(enum log_level level) const
    {
    return (level >= m_log_level);
    }





// function unlocks the object so that operations on it are thread safe.

inline logger &logger::unlock()
    {
    pthread_mutex_unlock(&m_lock);
    return (*this);
    }




// this class is a dummy class to allow the hexdump() manipulator to function correctly.
// Sometimes this technique for manipulator implementation is called an effector.

class hexdump
    {
public:
    hexdump(const void *src, int len, int width);                           // constructor defines how manipulator looks
    friend std::ostream &operator<<(std::ostream &os, const hexdump &hd);   // defines the logger operator overload
private:
    // The line below has been removed as under GCC 3.4.4 the copy constructor appears to be considered
    // called when the hexdump is printed in the LOG macro
    // hexdump(const hexdump &);                                            // copy constructor, DO NOT IMPLEMENT
    hexdump &operator=(const hexdump &);                                    // assignment operator, DO NOT IMPLEMENT
    const char          *m_src;                                             // ptr to buffer to hexdump
    int                 m_len;                                              // length of buffer in bytes
    int                 m_width;                                            // width in bytes of hexdump
    };





// Macro designed to be called by the application when it has data to log. Must be supplied
// with a valid log object, a log level and a log out line. It then makes the call MT safe
// and calls internal log routines to perform the work. A sample LOG call may look like
//
// LOG(l, logger::warning, name, "a bit of data " << 12 << 132.45 << "more text to output" << "\n")
//
// Note do not terminate statement with ";" it is not required.
// Really its just simplifies the writing of log statements. You can do it all longhand if you
// prefer, but make sure you nest your code within lock() / unlock() statments to be MT safe.
// Basically the log data is written into the strstream raw, then a log routine is called to
// proces the data.


#define LOG(L, NAME, LOG_LEVEL, X) \
    { \
    (L).lock(); \
    if ((L).logging_required(LOG_LEVEL)) \
        { \
        (L) << X; \
        (L).process_data(NAME, LOG_LEVEL); \
        } \
   (L).unlock(); \
    }


