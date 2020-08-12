#pragma once


#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sys/time.h>
#include <string>





class timestamp
    {
public:
    timestamp();
    enum time_type
        {
        gmt,
        local
        };
    const char *hmsu_today(enum time_type value = local);
    const char *ymdhmsu_today(enum time_type value = local);
    long usecs_today(enum time_type value = local);
    static long usecs_today_slow(time_type value = local);
    static inline unsigned long rdtsc64_sync();
    static inline unsigned long rdtsc64_nosync();
    static inline void rdtsc_ticks_delay(unsigned int num);
    static unsigned long calibrate_rdtsc_ticks_over_usec_period(unsigned long period_in_usecs);
    static int hours_diff_to_utc();
    static inline const char *ymdhmsu_today_slow(char *dst, time_type value = local);
    static inline const char *ymdhmsu_today_slow2(char *dst, time_type value = local);
    static inline long base_epoch_usecs();
    static inline long base_epoch_msecs();
    static inline long today_epoch_usecs();
    static inline long today_epoch_msecs();
    static inline long today_epoch_secs();
    static inline long today_usecs_from_epoch(long epoch_usecs);
    static inline long epoch_usecs();
    static inline long epoch_msecs();
    static inline long epoch_secs();
    static inline char *secs_to_cstring(char *dst, const int ts);
    static inline char *msecs_to_cstring(char *dst, const int ts);
    static inline char *usecs_to_cstring(char *dst, const long ts);
    static inline char *nsecs_to_cstring(char *dst, const long ts);
    static inline std::string secs_to_string(const int ts);
    static inline std::string msecs_to_string(const int ts);
    static inline std::string usecs_to_string(const long ts);
    static inline std::string nsecs_to_string(const long ts);
    static inline char *usecs_to_ascii_17_digits(char *dst, const long usecs, bool null_terminate = false);
    static inline int msecs_from_hhmmssmmm(const char *src);
    static inline long usecs_from_hhmmss(const char *src);              // HH:MM:SS
    static inline long usecs_from_hhmmssmmm(const char *src);           // HH:MM:SS.MMM
    static inline long usecs_from_hhmmssuuuuuu(const char *src);        // HH:MM:SS.UUUUUU
    static inline int mins_from_hhmm(const char *src);                  // HH:MM
    static inline int secs_from_hhmmss(const char *src);                // HH:MM:SS
    static inline int mins_from_hhmm(const std::string &src);           // HH:MM
    static inline int secs_from_hhmmss(const std::string &src);         // HH:MM:SS
    static const int            m_timestamp_len = 15;                   // HH:MM:SS.UUUUUU
    static const int            mf_timestamp_len = 24;                  // YYYYMMDD-HH:MM:SS.UUUUUU
    static const int            usecs_to_cstring_length = 15;           // HH:MM:SS.UUUUUU
    static const int            usecs_to_ascii_17_digits_length = 17;   // NNNNNNNNNNNNNNNNN
    static inline bool validate_hhmm(const std::string &time);
    static inline bool validate_hhmm(const char *value);
    static inline bool validate_hhmmss(const std::string &time);
    static inline bool validate_hhmmss(const char *value);
    static int get_day_of_year(time_type value = local);
    static int get_day_of_year(std::string &date);
private:
    timestamp(const timestamp &);
    timestamp &operator=(const timestamp &);
    time_t                      m_time;                                 // saved system time in secs
    time_t                      m_usecs;                                // saved system time in usecs
    char                        m_timestamp[m_timestamp_len + 1];       // big enough to hold time HH:MM:SS.UUUUUU + '\0'
    time_type                   m_last_type_called;                     // records the last type of timestamp call
    time_t                      mf_time;                                // saved system time in secs
    time_t                      mf_usecs;                               // saved system time in usecs
    char                        mf_timestamp[mf_timestamp_len + 1];     // big enough to hold time HH:MM:SS.UUUUUU + '\0'
    time_type                   mf_last_type_called;                    // records the last type of timestamp call
    time_t                      ml_time;                                // saved system time in secs
    long                        ml_daysec;                              // second of the day
    time_type                   ml_last_type_called;                    // records the last type of timestamp call
    };





// used to read the RDTSC 64 bit register. This register is reset to 0 on boot, then increments at the CPU clock rate forever.
// will take hundreds of years to wrap so dont worry about it. haha.
// this can be used to time very low latency events, it ticks with the frequency of the CPU clock.

inline unsigned long timestamp::rdtsc64_sync()
    {
    unsigned int lo, hi;
    asm volatile(
        "cpuid\n"
        "rdtsc"
        : "=a"(lo), "=d"(hi)
        : "a"(0)
        : "%rbx", "%rcx");
    return (((unsigned long)hi << 32) | (unsigned long)lo);
    }



inline unsigned long timestamp::rdtsc64_nosync()
    {
    unsigned int lo, hi;
    asm volatile(
//        "cpuid\n"
        "rdtsc"
        : "=a"(lo), "=d"(hi)
        : "a"(0)
        : "%rbx", "%rcx");
    return (((unsigned long)hi << 32) | (unsigned long)lo);
    }




inline void timestamp::rdtsc_ticks_delay(unsigned int num)
    {
    unsigned long end = timestamp::rdtsc64_nosync() + num;

    while (timestamp::rdtsc64_nosync() < end)
        ;
    return;
    }








inline char *timestamp::secs_to_cstring(char *dst, const int ts)
    {
    const int hrs = ts / (60 * 60);
    const int mins = (ts - (hrs * 60 * 60)) / 60;
    const int secs = (ts - (hrs * 60 * 60) - (mins * 60));

    dst[2] = dst[5] = ':';
    dst[0] = (hrs / 10) + '0';
    dst[1] = (hrs % 10) + '0';
    dst[3] = (mins / 10) + '0';
    dst[4] = (mins % 10) + '0';
    dst[6] = (secs / 10) + '0';
    dst[7] = (secs % 10) + '0';
    dst[8] = 0;
    return (dst);
    }





inline char *timestamp::msecs_to_cstring(char *dst, const int ts)
    {
    const int hrs = ts / (1000 * 60 * 60);
    const int mins = (ts - (hrs * 1000 * 60 * 60)) / (1000 * 60);
    const int secs = (ts - (hrs * 1000 * 60 * 60) - (mins * 1000 * 60)) / 1000;
    int millis = (ts - (hrs * 1000 * 60 * 60) - (mins * 1000 * 60) - secs * 1000);

    dst[2] = dst[5] = ':';
    dst[8] = '.';
    dst[0] = (hrs / 10) + '0';
    dst[1] = (hrs % 10) + '0';
    dst[3] = (mins / 10) + '0';
    dst[4] = (mins % 10) + '0';
    dst[6] = (secs / 10) + '0';
    dst[7] = (secs % 10) + '0';
    dst[11] = (millis % 10) + '0';
    millis /= 10;
    dst[10] = (millis % 10) + '0';
    dst[9] = (millis / 10) + '0';
    dst[12] = 0;
    return (dst);
    }





inline char *timestamp::usecs_to_cstring(char *dst, const long ts)
    {
    const long hrs = ts / (1000000L * 60 * 60);
    const long mins = (ts - (hrs * 1000000L * 60 * 60)) / ( 1000000L * 60);
    const long secs = (ts - (hrs * 1000000L * 60 * 60) - (mins * 1000000 * 60)) / 1000000;
    long usecs = (ts - (hrs * 1000000L * 60 * 60) - (mins * 1000000 * 60) - secs * 1000000);

    dst[2] = dst[5] = ':';
    dst[8] = '.';
    dst[0] = (hrs / 10) + '0';
    dst[1] = (hrs % 10) + '0';
    dst[3] = (mins / 10) + '0';
    dst[4] = (mins % 10) + '0';
    dst[6] = (secs / 10) + '0';
    dst[7] = (secs % 10) + '0';
    dst[14] = (usecs % 10) + '0';
    usecs /= 10;
    dst[13] = (usecs % 10) + '0';
    usecs /= 10;
    dst[12] = (usecs % 10) + '0';
    usecs /= 10;
    dst[11] = (usecs % 10) + '0';
    usecs /= 10;
    dst[10] = (usecs % 10) + '0';
    dst[9] = (usecs / 10) + '0';
    dst[15] = 0;
    return (dst);
    }





inline char *timestamp::nsecs_to_cstring(char *dst, const long ts)
    {
    const long hrs = ts / (1000000000L * 60 * 60);
    const long mins = (ts - (hrs * 1000000000L * 60 * 60)) / (1000000000L * 60);
    const long secs = (ts - (hrs * 1000000000L * 60 * 60) - (mins * 1000000000L * 60)) / 1000000000L;
    long nsecs = (ts - (hrs * 1000000000L * 60 * 60) - (mins * 1000000000L * 60) - secs * 1000000000L);

    dst[0] = (hrs / 10) + '0';
    dst[1] = (hrs % 10) + '0';
    dst[2] = ':';
    dst[3] = (mins / 10) + '0';
    dst[4] = (mins % 10) + '0';
    dst[5] = ':';
    dst[6] = (secs / 10) + '0';
    dst[7] = (secs % 10) + '0';
    dst[8] = '.';
    dst[17] = (nsecs % 10) + '0';
    nsecs /= 10;
    dst[16] = (nsecs % 10) + '0';
    nsecs /= 10;
    dst[15] = (nsecs % 10) + '0';
    nsecs /= 10;
    dst[14] = (nsecs % 10) + '0';
    nsecs /= 10;
    dst[13] = (nsecs % 10) + '0';
    nsecs /= 10;
    dst[12] = (nsecs % 10) + '0';
    nsecs /= 10;
    dst[11] = (nsecs % 10) + '0';
    nsecs /= 10;
    dst[10] = (nsecs % 10) + '0';
    dst[9] = (nsecs / 10) + '0';
    dst[18] = 0;
    return (dst);
    }



inline std::string timestamp::secs_to_string(const int ts)
    {
    char dst[32];

    secs_to_cstring(dst, ts);
    return (std::string(dst));
    }


inline std::string timestamp::msecs_to_string(const int ts)
    {
    char dst[32];

    msecs_to_cstring(dst, ts);
    return (std::string(dst));
    }

inline std::string timestamp::usecs_to_string(const long ts)
    {
    char dst[32];

    usecs_to_cstring(dst, ts);
    return (std::string(dst));
    }

inline std::string timestamp::nsecs_to_string(const long ts)
    {
    char dst[32];

    nsecs_to_cstring(dst, ts);
    return (std::string(dst));
    }








// returns the base time in usecs of 00:00:00.000000 today from the Epoch.
// the "now" time in usecs can then be added to it to form a complete time
// in usecs from the epoch

inline long timestamp::base_epoch_usecs()
    {
    struct timeval tv;

    gettimeofday(&tv, 0);
    return (tv.tv_sec / 86400L * 86400000000L);
    }

inline long timestamp::base_epoch_msecs()
    {
    return (base_epoch_usecs() / 1000L);
    }



inline long timestamp::today_usecs_from_epoch(long epoch_usecs)
    {
    return (epoch_usecs % 86400000000L);
    }





inline long timestamp::today_epoch_usecs()
    {
    struct timeval tv;

    gettimeofday(&tv, 0);
    return ((tv.tv_sec % (24 * 60 * 60L)) * 1000000L + tv.tv_usec);
    }

inline long timestamp::today_epoch_msecs()
    {
    return (today_epoch_usecs() / 1000L);
    }

inline long timestamp::today_epoch_secs()
    {
    return (today_epoch_usecs() / 1000000L);
    }








inline long timestamp::epoch_usecs()
    {
    struct timeval tv;

    gettimeofday(&tv,0);
    return (tv.tv_sec * 1000000L + tv.tv_usec);
    }

inline long timestamp::epoch_msecs()
    {
    return (epoch_usecs() / 1000L);
    }


inline long timestamp::epoch_secs()
    {
    struct timeval tv;

    gettimeofday(&tv,0);
    return (tv.tv_sec);
    }





// 17 digits is big enough to store epoch in usecs with 1 digit to spare.

inline char *timestamp::usecs_to_ascii_17_digits(char *dst, const long usecs, bool null_terminate)
    {
    char tmp[32];

    sprintf(tmp, "%017ld", usecs);
    memcpy(dst, tmp, usecs_to_ascii_17_digits_length);
    if (null_terminate == true)
        dst[17] = 0;
    return (dst);
    }




// this function expects HH:MM:SS.MMM as input.

inline int timestamp::msecs_from_hhmmssmmm(const char *src)
    {
    return (((src[0] - '0') * 10 + (src[1] - '0')) * 3600000 + ((src[3] - '0') * 10 + (src[4] - '0')) * 60000 + ((src[6] - '0') * 10 + (src[7] - '0')) * 1000 + ((src[9] - '0') * 100 + (src[10] - '0') * 10 + (src[11] - '0')));
    }



// this function expects HH:MM:SS as input.

inline long timestamp::usecs_from_hhmmss(const char *src)
    {
    return (((src[0] - '0') * 10 + (src[1] - '0')) * 3600000000L + ((src[3] - '0') * 10 + (src[4] - '0')) * 60000000L + ((src[6] - '0') * 10 + (src[7] - '0')) * 1000000L);
    }




// this function expects HH:MM as input.

inline int timestamp::mins_from_hhmm(const std::string &src)
    {
    return (mins_from_hhmm(src.c_str()));
    }


// this function expects HH:MM as input.

inline int timestamp::mins_from_hhmm(const char *src)
    {
    return (((src[0] - '0') * 10 + (src[1] - '0')) * 60 + ((src[3] - '0') * 10 + (src[4] - '0')));
    }



// this function expects HH:MM:SS as input.

inline int timestamp::secs_from_hhmmss(const std::string &src)
    {
    return (secs_from_hhmmss(src.c_str()));
    }


// this function expects HH:MM:SS as input.

inline int timestamp::secs_from_hhmmss(const char *src)
    {
    return (((src[0] - '0') * 10 + (src[1] - '0')) * 3600 + ((src[3] - '0') * 10 + (src[4] - '0')) * 60 + ((src[6] - '0') * 10 + (src[7] - '0')));
    }



// this function expects HH:MM:SS.MMM as input.

inline long timestamp::usecs_from_hhmmssmmm(const char *src)
    {
    return (msecs_from_hhmmssmmm(src) * 1000L);
    }





// this function expects HH:MM:SS.UUUUUU as input.

inline long timestamp::usecs_from_hhmmssuuuuuu(const char *src)
    {
    return (((src[0] - '0') * 10 + (src[1] - '0')) * 3600000000L + ((src[3] - '0') * 10 + (src[4] - '0')) * 60000000L + ((src[6] - '0') * 10 + (src[7] - '0')) * 1000000L +
        ((src[9] - '0') * 100000L + (src[10] - '0') * 10000L + (src[11] - '0') * 1000L +(src[12] - '0') * 100L + (src[13] - '0') * 10L + (src[14] - '0')));
    }




inline bool timestamp::validate_hhmm(const std::string &value)
    {
    return (validate_hhmm(value.c_str()));
    }




inline bool timestamp::validate_hhmm(const char *value)
    {
    int h, m;

    if (value && *value && strlen(value) == 5 && value[2] == ':' && isdigit(value[0]) && isdigit(value[1]) &&
        isdigit(value[3]) && isdigit(value[4]))
        {
        h = (value[0] - '0') * 10 + (value[1] - '0');
        m = (value[3] - '0') * 10 + (value[4] - '0');
        return (h >= 0 && h <= 23 && m >= 0 && m <= 59);
        }
    return (false);
    }






inline bool timestamp::validate_hhmmss(const std::string &value)
    {
    return (validate_hhmmss(value.c_str()));
    }




inline bool timestamp::validate_hhmmss(const char *value)
    {
    int h, m, s;

    if (value && *value && strlen(value) == 8 && value[2] == ':' && value[5] == ':' && isdigit(value[0]) && isdigit(value[1]) &&
        isdigit(value[3]) && isdigit(value[4]) && isdigit(value[6]) && isdigit(value[7]))
        {
        h = (value[0] - '0') * 10 + (value[1] - '0');
        m = (value[3] - '0') * 10 + (value[4] - '0');
        s = (value[6] - '0') * 10 + (value[7] - '0');
        return (h >= 0 && h <= 23 && m >= 0 && m <= 59 && s >= 0 && s <= 59);
        }
    return (false);
    }






// function returns a pointer to a const C string containing a timestamp in the form
// YYYYMMDD-HH:MM:DD.UUUUUU

inline const char *timestamp::ymdhmsu_today_slow(char *dst, time_type value)
    {
    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, NULL);
    (value == local) ? localtime_r(&tv.tv_sec, &tm) : gmtime_r(&tv.tv_sec, &tm);
    sprintf(dst, "%4.4d%2.2d%2.2d-%2.2d:%2.2d:%2.2d.%6.6ld", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec);
    return (dst);
    }





// function returns a pointer to a const C string containing a timestamp in the form
// YYYYMMDD-HHMMDD.UUUUUU

inline const char *timestamp::ymdhmsu_today_slow2(char *dst, time_type value)
    {
    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, NULL);
    (value == local) ? localtime_r(&tv.tv_sec, &tm) : gmtime_r(&tv.tv_sec, &tm);
    sprintf(dst, "%4.4d%2.2d%2.2d-%2.2d%2.2d%2.2d.%6.6ld", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec);
    return (dst);
    }



