#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <h/logger.hpp>

using namespace std;



// data structure holds a list of pointers to text representation of the legal
// logging levels provided by the logger class. Also an array of lengths for each
// of the logging level strings. These are used for speed when placing the strings into
// the log buffer.

const char *logger::str_log_levels[] =
    {
    "Debug_9 ",
    "Debug_8 ",
    "Debug_7 ",
    "Debug_6 ",
    "Debug_5 ",
    "Debug_4 ",
    "Debug_3 ",
    "Debug_2 ",
    "Debug_1 ",
    "Debug_0 ",
    "Info    ",
    "Warning ",
    "Error   ",
    "Fatal   ",
    "Always  "
    };
const enum logger::log_level logger::enum_log_levels[] =
    {
    Debug_9,
    Debug_8,
    Debug_7,
    Debug_6,
    Debug_5,
    Debug_4,
    Debug_3,
    Debug_2,
    Debug_1,
    Debug_0,
    Info,
    Warning,
    Error,
    Fatal,
    Always
    };
const size_t logger::str_log_levels_len[] =
    {
    strlen(str_log_levels[Debug_9]),
    strlen(str_log_levels[Debug_8]),
    strlen(str_log_levels[Debug_7]),
    strlen(str_log_levels[Debug_6]),
    strlen(str_log_levels[Debug_5]),
    strlen(str_log_levels[Debug_4]),
    strlen(str_log_levels[Debug_3]),
    strlen(str_log_levels[Debug_2]),
    strlen(str_log_levels[Debug_1]),
    strlen(str_log_levels[Debug_0]),
    strlen(str_log_levels[Info]),
    strlen(str_log_levels[Warning]),
    strlen(str_log_levels[Error]),
    strlen(str_log_levels[Fatal]),
    strlen(str_log_levels[Always]),
    };




void *logger::thread_thunk(void *p)
    {
    logger *l = (logger *)p;

    l->disk_thread();
    return (0);
    }




// Constructor for the logger class. The base strstream class is initialised and all the class
// variables are setup. The log level is set to a default value and because the logger calls do their
// own locking, the strstream MT locking is disabled for performance. An out stream must always
// be provided but the log file is optional. Finally an initial timestamp is created.

logger::logger(ostream &out_stream, const char *file_name) : strstream(), m_out_stream(out_stream)
    {
    int ret;
    struct pool_entry e;

    m_file_name = 0;
    m_time = 0;
    m_usecs = 0;
    m_log_level = logger::Info;
    m_force_flush_log_level = logger::Warning;
    m_file_out = false;
    m_terminate = false;
    pthread_mutex_init(&m_lock, NULL);
    pthread_mutex_init(&m_io_lock, NULL);
    pthread_mutex_init(&m_write_mutex, NULL);
    pthread_cond_init(&m_write_cv, NULL);
    m_async = false;
    m_stream_out = true;
//    ios_base::sync_with_stdio(false);                                         // 20091103 MDW used this just for testing. may enable it one day
    if ((ret = pthread_create(&m_write_tid, NULL, thread_thunk, this)) != 0)    // pre-allocate everything for asynchronous writing
        {
        std::ostringstream o_str;

        o_str << "creating logger disk writer thread failed, error " << ret;
        throw std::runtime_error(o_str.str());
        }
    m_cptr = m_buf = new char[sz_log_buf + sz_log_buf_extra];
    e.buf = new char[sz_log_buf + sz_log_buf_extra];                            // allocate the next clean pool entry now, to avoid having to do this when m_buf fills up for the first time
    e.len = 0;                                                                  // keep the compiler happy
    m_clean_pool.push_back(e);
    if (file_name != 0)
        {
        m_file_name = new char [strlen(file_name) + 1];
        strcpy(m_file_name, file_name);
        m_file_out = true;
        }
    timestamp();
    enable_stream();
    enable_file();
    enable_buffered();
    return;
    }





// Destructor for logger class. It calls a function to write out any buffered data. Class
// resources are released and the object destroys itself.

logger::~logger()
    {
    lock();
    p_do_work();                                        // flush of any existing buffered data
    pthread_mutex_lock(&m_write_mutex);                 // get control of shared variable with disk thread
    m_terminate = true;                                 // request it terminate
    pthread_cond_signal(&m_write_cv);                   // wake up the disk writer thread so it can terminate before us
    pthread_mutex_unlock(&m_write_mutex);               // allow it to do work
    pthread_join(m_write_tid, NULL);                    // wait for the disk writer thread to terminate
    delete [] m_file_name;
    pthread_mutex_destroy(&m_io_lock);
    pthread_mutex_destroy(&m_write_mutex);
    pthread_cond_destroy(&m_write_cv);
    while (!m_clean_pool.empty())                       // clean up, delete clean buffers
        {
        struct pool_entry e = m_clean_pool.back();
        m_clean_pool.pop_back();
        delete [] e.buf;
        }
    while (!m_dirty_pool.empty())                       // clean up, delete dirty buffers
        {
        struct pool_entry e = m_dirty_pool.back();
        m_dirty_pool.pop_back();
        delete [] e.buf;
        }
    assert(m_dirty_pool.empty());
    assert(m_clean_pool.empty());
    assert(m_cptr == m_buf);                            // m_buf always points to an allocated buffer and m_cptr must equal m_buf, otherwise p_do_work() did not flush all the data out or some other corruption has occurred
    delete [] m_buf;
    unlock();
    pthread_mutex_destroy(&m_lock);
    return;
    }








// function disables buffered mode of operation and flushes out any remaining buffered data

void logger::disable_buffered()
    {
    lock();
    m_buffered = false;
    p_do_work();
    unlock();
    return;
    }






// function enables log output to the specified file

void logger::disable_file()
    {
    lock();
    if (m_file_name != 0)
        m_file_out = false;
    unlock();
    return;
    }





// function disables output to the specified stream

void logger::disable_stream()
    {
    lock();
    m_stream_out = false;
    unlock();
    return;
    }




// function disables asynchronous write mode of operation

void logger::disable_async()
    {
    bool is_empty = false;

    lock();
    p_do_work();
    m_async = false;
    while (is_empty == false)
        {
        pthread_mutex_lock(&m_write_mutex);
        is_empty = m_dirty_pool.empty();
        pthread_mutex_unlock(&m_write_mutex);
        usleep(10000L);                                             // sleep 10ms
        }
    unlock();
    return;
    }


// function enables asynchronous write mode of operation

void logger::enable_async()
    {
    lock();
    m_async = true;
    unlock();
    return;
    }





// function enables buffered mode of operation

void logger::enable_buffered()
    {
    lock();
    m_buffered = true;
    unlock();
    return;
    }





// function enables log output to the specified file

void logger::enable_file()
    {
    lock();
    if (m_file_name != 0)
        m_file_out = true;
    unlock();
    return;
    }




// function enables output to the specified stream

void logger::enable_stream()
    {
    lock();
    m_stream_out = true;
    unlock();
    return;
    }





// function explicitly flushes any buffered data. Should only be used when process is terminating or when
// data is required to be sent to disk. Could be done on 1 - 10 second timeout, for instance.

void logger::flush()
    {
    lock();
    p_do_work();
    unlock();
    return;
    }






// function returns the current log level

enum logger::log_level logger::get_force_flush_level()
    {
    enum log_level tmp;

    lock();
    tmp = m_force_flush_log_level;
    unlock();
    return (tmp);
    }





// function returns the current log level

enum logger::log_level logger::get_level()
    {
    enum log_level tmp;

    lock();
    tmp = m_log_level;
    unlock();
    return (tmp);
    }




const char *logger::get_str_level()
    {
    enum log_level level;

    lock();
    level = m_log_level;
    unlock();
    return (get_str_level(level));
    }






bool logger::get_level_from_str(enum log_level &level, const char *str)
    {
    unsigned int i;

    for (i = 0; i < sizeof(str_log_levels) / sizeof(str_log_levels[0]); i++)
        if (!strncmp(str_log_levels[i], str, strlen(str)))
            {
            level = enum_log_levels[i];
            return (true);
            }
    return (false);
    }







// function processes the log data that has been read into the strstream buffer. Basically it writes
// log lines in the format "YYYYMMDD HH:MM:SS.UUUUUU <log_level> <app name> <formatted text>
// Every log line has a header and any embedded '\n' '\r' '\0' characters in the data stream are treated
// as generating a new log line. If the internal data buffer becomes full then a function is called to
// write the data to the appropriate places. Once all the strstream data has been processed then a function
// is called to see if the residual data can stay in the buffer or has to be written immediately. Any log data
// that does not have a terminating '\n' has one appended automatically.
// To be able to access the strstream streambuf directly the buffer must be frozen first then unfrozen at the
// end of the processing. Before unfreezing the stream put pointer is reset to 0.
// Note that the use of the m_timestamp_len which has a constant length. If the format of the timestamp is changed
// this will need to be updated as well.

void logger::process_data(const string &name, enum log_level level)
    {
    long len;
    char *sp, c = '\n';
    int level_len = get_str_level_len(level);
    const char *str_level = get_str_level(level);
    const char *namestr = name.c_str();
    int namesize = name.size();

    if ((len = tellp()) > 0)
        {
        sp = rdbuf()->str();
        while (len--)
            {
            if (c == '\n')
                {
                memcpy(m_cptr, timestamp(), m_timestamp_len);
                m_cptr += m_timestamp_len;
                memcpy(m_cptr, str_level, level_len);
                m_cptr += level_len;
                *m_cptr++ = ' ';
                memcpy(m_cptr, namestr, namesize);
                m_cptr += namesize;
                *m_cptr++ = ' ';
                }
            if ((c = *sp++) == '\r' || c == '\0')
                c = '\n';
            *m_cptr++ = c;
            if (m_cptr - m_buf >= sz_log_buf)
                p_do_work();
            }
        if (c != '\n')
            *m_cptr++ = '\n';
        len = m_cptr - m_buf;
        if ((m_buffered == false && len != 0) || len >= sz_log_buf)
            p_do_work();
        seekp(0);
        rdbuf()->freeze(0);
        }
    if (m_force_flush_log_level <= m_log_level && m_buffered == true)
        p_do_work();
    return;
    }






// function sets a new force log flush level

enum logger::log_level logger::set_force_flush_level(enum log_level new_level)
    {
    lock();
    m_force_flush_log_level = new_level;
    unlock();
    return (new_level);
    }





// function sets a new log level

enum logger::log_level logger::set_level(enum log_level new_level)
    {
    lock();
    m_log_level = new_level;
    unlock();
    return (new_level);
    }






// function sets a new log level. The supplied string must match one of the predefined
// log levels, or the function will leave the log level at its previous value.

enum logger::log_level logger::set_level(const char *ll_str)
    {
    enum log_level tmp;
    unsigned int i;

    lock();
    for (i = 0; i < sizeof(str_log_levels) / sizeof(str_log_levels[0]); i++)
        if (!strncmp(str_log_levels[i], ll_str, strlen(ll_str)))
            {
            m_log_level = enum_log_levels[i];
            break;
            }
    tmp = m_log_level;
    unlock();
    return (tmp);
    }





// function sets a new filename up to write to, but first flushes out any buffered
// data if it exists. The new filename is not tested for correctness. If it is wrong
// data will fail to be written to disk. The state of the m_file_out flag is saved, and
// this state is reinstated if the new filename is deemed to not be a 0 length string.

void logger::set_new_file(const char *file_name)
    {
    int len;
    bool old_file_out;

    lock();
    p_do_work();
    pthread_mutex_lock(&m_io_lock);
    delete [] m_file_name;
    m_file_name = 0;
    old_file_out = m_file_out;
    m_file_out = false;
    if ((len = strlen(file_name)) > 0)
        {
        m_file_name = new char [len + 1];
        strcpy(m_file_name, file_name);
        m_file_out = old_file_out;
        }
    pthread_mutex_unlock(&m_io_lock);
    unlock();
    return;
    }




// function logs the current timestamp in the form "YYYYMMDD HHMMSS.TTT " Because of the really slow
// performance of localtime_r, the timestamp is only setup once the time has changed by at least one
// second, so the time consuming work will only be done at a maximum rate of 1 call per second.
// The buffer is held in the log object for later output.
// For performance the length of the current timestamp in bytes is saved as well.
// Also for performance if the thousandths of a second change between calls, then the new thousandths
// will be placed back into the buffer by hand. Since this operation is expensive it is hand coded
// to be fast. So change it and the length of the time string carefully.
// NOTE
// time() is a system call, gettimeofday() isnt, so no context switching, much much faster.
// this must only be called while nested within lock() / unlock() pair.

const char *logger::timestamp()
    {
    struct timeval tv;
    struct tm tm;
    int tmp;

    gettimeofday(&tv, NULL);
    if (tv.tv_sec != m_time)
        {
        m_time = tv.tv_sec;
        m_usecs = tv.tv_usec;
        localtime_r(&m_time, &tm);
        sprintf(m_timestamp, "%4.4d%2.2d%2.2d %2.2d:%2.2d:%2.2d.%6.6ld ", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec);
        }
    else if ((tmp = tv.tv_usec) != m_usecs)
        {
        m_timestamp[23] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[22] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[21] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[20] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[19] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[18] = (tmp % 10) + '0';
        m_usecs = tv.tv_usec;
        }
    return (m_timestamp);
    }






// write data to disk
// only 1 thread at a time can enter.
// this is only called for main logger thread.

void logger::send_to_files(const char *buf, int len)
    {
    int fd, num;

    pthread_mutex_lock(&m_io_lock);
    if (len && (fd = ::open(m_file_name, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0)
        {
        while (len)
            {
            if ((num = ::write(fd, buf, len)) > 0)
                {
                buf += num;
                len -= num;
                }
            else if (errno != EINTR && errno != EAGAIN)
                break;
            }
        ::close(fd);
        }
    pthread_mutex_unlock(&m_io_lock);
    return;
    }




// write data to disk
// this is called from the async disk thread and deals with a queue of log buffers waiting to be written to disk.
// In this case the disk file is only opened once or all the buffers to be written. Bit more efficient that way.
// Also as each buffer is written out it is returned to the clean pool as soon as possible to keep memory usage to
// a minimum.

void logger::send_to_files(std::deque<struct pool_entry> &waiting_pool)
    {
    std::deque<struct pool_entry>::iterator pos;
    int fd, num, len;
    char *src;

    pthread_mutex_lock(&m_io_lock);
    if ((fd = ::open(m_file_name, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0)
        {
        for (pos = waiting_pool.begin(); pos != waiting_pool.end(); ++pos)
            {
            src = pos->buf;
            len = pos->len;
            while (len)
                {
                if ((num = ::write(fd, src, len)) > 0)
                    {
                    src += num;
                    len -= num;
                    }
                else if (errno != EINTR && errno != EAGAIN)
                    break;
                }
            }
        ::close(fd);
        }
    pthread_mutex_unlock(&m_io_lock);
    return;
    }






// only enter cond wait if there are no more log buffers to process.

void logger::disk_thread()
    {
    std::deque<struct pool_entry> waiting_pool;

    sigset_t s;
    sigfillset(&s);
    pthread_sigmask(SIG_BLOCK, &s, NULL);
    pthread_mutex_lock(&m_write_mutex);
    while (m_terminate == false)                                    // before going to sleep, check for termination
        {
        if (m_dirty_pool.empty() == true)
            pthread_cond_wait(&m_write_cv, &m_write_mutex);             // must call this with mutex LOCKED, returns with mutex LOCKED
        while (m_dirty_pool.empty() == false)                       // loop while more data to write to disk
            {
            waiting_pool = m_dirty_pool;
            m_dirty_pool.clear();
            pthread_mutex_unlock(&m_write_mutex);                   // do not hold the lock while doing IO
            send_to_files(waiting_pool);
            pthread_mutex_lock(&m_write_mutex);
            m_clean_pool.insert(m_clean_pool.end(), waiting_pool.begin(), waiting_pool.end());
            waiting_pool.clear();
            }
        }
    pthread_mutex_unlock(&m_write_mutex);
    return;
    }

// MDW 12/10/2013.
// code below has subtle problem where if main thread has written the first line to it in async mode and then disabled async
// mode BEFORE the disk write thread has actually had a chance to start (window of usecs), thereby sitting in cond wait even
// though there is a buffer to process in dirty pool. You have to engineer this to happen as once the thread is started its fine.
// So code was changed to the above, which is better and correct.

#if 0
void logger::disk_thread()
    {
    std::deque<struct pool_entry> waiting_pool;

    sigset_t s;
    sigfillset(&s);
    pthread_sigmask(SIG_BLOCK, &s, NULL);
    pthread_mutex_lock(&m_write_mutex);
    while (m_terminate == false)                                    // before going to sleep, check for termination
        {
        pthread_cond_wait(&m_write_cv, &m_write_mutex);             // must call this with mutex LOCKED, returns with mutex LOCKED
        while (m_dirty_pool.empty() == false)                       // loop while more data to write to disk
            {
            waiting_pool = m_dirty_pool;
            m_dirty_pool.clear();
            pthread_mutex_unlock(&m_write_mutex);                   // do not hold the lock while doing IO
            send_to_files(waiting_pool);
            pthread_mutex_lock(&m_write_mutex);
            m_clean_pool.insert(m_clean_pool.end(), waiting_pool.begin(), waiting_pool.end());
            waiting_pool.clear();
            }
        }
    pthread_mutex_unlock(&m_write_mutex);
    return;
    }
#endif



// function checks to see if the output stream and file are enabled for logging and if so,
// just writes the log data from the internal buffer. Once written the buffer pointer is
// reset back to the start ready for more data. Some streams are set in buffered mode and
// since the log class does its own buffering the code makes sure that any stream data
// is always flushed out.

void logger::p_do_work()
    {
    struct pool_entry e;
    int len;

    if ((len = m_cptr - m_buf) > 0)
        {
        if (m_stream_out == true)
            {
            m_out_stream.write(m_buf, len);                             // 20091103 MDW sometimes this errors for some unknown reason. When output is buffered. Dont know why.
            m_out_stream.flush();                                       // so for now just clear the error if one exists. Dont have time to investigate. Possibly trying to write
            if (!m_out_stream)                                          // too much in one go. Testing is difficult as this does not happen every time.
                m_out_stream.clear();
            }
        if (m_file_out == true)
            {
            if (m_async == true)
                {
                e.buf = m_buf;
                e.len = len;
                pthread_mutex_lock(&m_write_mutex);
                m_dirty_pool.push_back(e);
                if (m_clean_pool.empty())
                    m_buf = new char[sz_log_buf + sz_log_buf_extra];
                else
                    {
                    e = m_clean_pool.back();
                    m_buf = e.buf;
                    m_clean_pool.pop_back();
                    }
                pthread_cond_signal(&m_write_cv);
                pthread_mutex_unlock(&m_write_mutex);
                }
            else
                send_to_files(m_buf, len);
            }
        m_cptr = m_buf;                                                 // reset the current pointer on exit m_cptr always equals m_buf (destructor relies on this for assert)
        }
    return;
    }








// function is the constructor for a dummy class that allow allows a hexdump() manipulator to be
// constructed so that hex dumping data can be done from within the logger. Basically all it does
// is to save its arguments away for the operator overloading function to use next. This whole thing
// is sometimes described as an effector.
// the constructor does not return anything.

hexdump::hexdump(const void *src, int len, int width) : m_src((const char *)src), m_len(len), m_width(width)
    {
    return;
    }




// function implements an operator overload for the logger. This function assumes the hexdump constructor
// has been called and proceeds to dump its input buffer in hex and ASCII to the ostream. First it builds
// up as much as it can in its internal buffer for performance reasons and when full, dumps it to the ostream.
// the function is hand coded for high performance and is extremely fast. This ensures that even when
// hexdumping lots of data, the logger does not slow the process down unduly.
// The calculation of max_chars is based on a fixed format of:
// 4 display characters per character to output * the display width
// + 3 spaces between the hex and ASCII displays
// + 2 boundary characters ||
// + 1 newline character
// 41 41 41 41 41 41 41 41   |AAAAAAAA|<nl>
//
// Do not change this format without changing the calculation to match, otherwise memory may be corrupted.

ostream &operator<<(ostream &os, const hexdump &hd)
    {
    static char myhex[] = "0123456789ABCDEF";
    unsigned char dst[16384];
    int fill_len, max_chars, xlen;
    unsigned char *h = 0, *a, *tmp = 0, c;
    const char *src = hd.m_src;
    int len = hd.m_len;

    os << "HexDump len=" << len << ", width=" << hd.m_width << endl;
    max_chars = (sizeof(dst) / (hd.m_width * 4 + 6)) * hd.m_width;
    while (len)
        {
        fill_len = (len > max_chars) ? max_chars : len;
        len -= fill_len;
        a = dst;
        while (fill_len)
            {
            h = a;
            tmp = a = h + hd.m_width * 3;
            *a++ = ' ';
            *a++ = ' ';
            *a++ = '|';
            xlen = (fill_len > hd.m_width) ? hd.m_width : fill_len;
            fill_len -= xlen;
            while (xlen--)
                {
                *h++ = myhex[(c = *src++) >> 4];
                *h++ = myhex[c & 0x0F];
                *h++ = ' ';
                *a++ = (c >= 0x20 && c <= 0x7E) ? c : '.';
                }
            *a++ = '|';
            *a++ = '\n';
            }
        while (h != tmp)
            *h++ = ' ';
        *a++ = '\0';
        os << dst;
        }
    return (os);
    }
