#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <h/timestamp.hpp>






// constructor, just initialises the members so that on the first call to
// get_timestamp, they will be updated.

timestamp::timestamp()
    {
    m_time = 0;
    m_usecs = 0;
    m_last_type_called = local;
    memset(m_timestamp, 0, sizeof(m_timestamp));
    mf_time = 0;
    mf_usecs = 0;
    mf_last_type_called = local;
    memset(mf_timestamp, 0, sizeof(mf_timestamp));
    ml_time = 0;
    ml_daysec = 0;
    ml_last_type_called = local;
    return;
    }






// function returns a pointer to a const C string containing a timestamp in the form
// HH:MM:DD.UUUUUU The function only recalculates the local/gmt time if the new call time differs by at least 1 sec.
// If the sec is the same the usecs are updated manually.
// We do this because calling localtime or gmtime is bloody slow, on the test hardware we would get 750000 calls a sec rather than 25000000
// Because data is persisted between calls it is not thread safe
// We could replace sprintf with hand conversion but it isnt it that takes the time up

const char *timestamp::hmsu_today(time_type value)
    {
    struct timeval tv;
    struct tm tm;
    int tmp;

    gettimeofday(&tv, NULL);
    if (tv.tv_sec != m_time || m_last_type_called != value)
        {
        m_last_type_called = value;
        m_time = tv.tv_sec;
        m_usecs = tv.tv_usec;
        (value == local) ? localtime_r(&m_time, &tm) : gmtime_r(&m_time, &tm);
        sprintf(m_timestamp, "%2.2d:%2.2d:%2.2d.%6.6ld", tm.tm_hour, tm.tm_min, tm.tm_sec, m_usecs);
        }
    else if ((tmp = tv.tv_usec) != m_usecs)
        {
        m_usecs = tmp;
        m_timestamp[14] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[13] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[12] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[11] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[10] = (tmp % 10) + '0';
        tmp /= 10;
        m_timestamp[9] = (tmp % 10) + '0';
        }
    return (m_timestamp);
    }






// function returns a pointer to a const C string containing a timestamp in the form
// YYYYMMDD-HH:MM:DD.UUUUUU The function only recalculates the local/gmt time if the new call time differs by at least 1 sec.
// If the sec is the same the usecs are updated manually.
// We do this because calling localtime or gmtime is bloody slow, on the test hardware we would get 750000 calls a sec rather than 25000000
// Because data is persisted between calls it is not thread safe
// We could replace sprintf with hand conversion but it isnt it that takes the time up

const char *timestamp::ymdhmsu_today(time_type value)
    {
    struct timeval tv;
    struct tm tm;
    int tmp;

    gettimeofday(&tv, NULL);
    if (tv.tv_sec != mf_time || mf_last_type_called != value)
        {
        mf_last_type_called = value;
        mf_time = tv.tv_sec;
        mf_usecs = tv.tv_usec;
        (value == local) ? localtime_r(&mf_time, &tm) : gmtime_r(&mf_time, &tm);
        sprintf(mf_timestamp, "%4.4d%2.2d%2.2d-%2.2d:%2.2d:%2.2d.%6.6ld", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, mf_usecs);
        }
    else if ((tmp = tv.tv_usec) != mf_usecs)
        {
        mf_usecs = tmp;
        mf_timestamp[23] = (tmp % 10) + '0';
        tmp /= 10;
        mf_timestamp[22] = (tmp % 10) + '0';
        tmp /= 10;
        mf_timestamp[21] = (tmp % 10) + '0';
        tmp /= 10;
        mf_timestamp[20] = (tmp % 10) + '0';
        tmp /= 10;
        mf_timestamp[19] = (tmp % 10) + '0';
        tmp /= 10;
        mf_timestamp[18] = (tmp % 10) + '0';
        }
    return (mf_timestamp);
    }





long timestamp::usecs_today_slow(time_type value)
    {
    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, NULL);
    (value == local) ? localtime_r(&tv.tv_sec, &tm) : gmtime_r(&tv.tv_sec, &tm);
    return ((tm.tm_hour * 3600L + tm.tm_min * 60L + tm.tm_sec) * 1000000L + tv.tv_usec);
    }




int timestamp::get_day_of_year(time_type value)
    {
    time_t t;
    struct tm zz = { };

    t = time(0);
    (value == local) ? localtime_r(&t, &zz) : gmtime_r(&t, &zz);
    return (zz.tm_yday);
    }



// date should be DD/MM

int timestamp::get_day_of_year(std::string &date)
    {
    struct tm zz = { };

    if (date.size() != 5)
        return (-1);
    zz.tm_mday = atoi(date.substr(0, 2).c_str());
    zz.tm_mon = atoi(date.substr(3, 2).c_str());
    if (zz.tm_mday < 1 || zz.tm_mday > 31 || zz.tm_mon < 1 || zz.tm_mon > 12)
        return (-1);
    --zz.tm_mon;
    if (mktime(&zz) == -1)
        return (-1);
    return (zz.tm_yday);
    }







// function returns a long containing a todays timestamp in usecs
// function only recalculates the local/gmt time if the new call time differs by at least 1 sec.
// If the sec is the same the usecs are updated manually.
// We do this because calling localtime or gmtime is bloody slow, on the test hardware we would get 750000 calls a sec rather than 25000000
// Because data is persisted between calls it is not thread safe

long timestamp::usecs_today(time_type value)
    {
    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, NULL);
    if (tv.tv_sec != ml_time || ml_last_type_called != value)
        {
        ml_last_type_called = value;
        ml_time = tv.tv_sec;
        (value == local) ? localtime_r(&ml_time, &tm) : gmtime_r(&ml_time, &tm);
        ml_daysec = tm.tm_hour * 3600L + tm.tm_min * 60L + tm.tm_sec;
        }
    return (ml_daysec * 1000000L + tv.tv_usec);
    }




// function calculates the difference in hours from UTC(GMT) and local time.
// the result is -ve if GMT behind local time or +ve if GMT ahead of local time.
// It also takes care of year end wrap round.

int timestamp::hours_diff_to_utc()
    {
    struct timeval tv;
    struct tm utc;
    struct tm local;

    gettimeofday(&tv, 0);
    gmtime_r(&tv.tv_sec, &utc);
    localtime_r(&tv.tv_sec, &local);
    if (utc.tm_year > local.tm_year)
        utc.tm_yday = local.tm_yday + 1;
    else if (utc.tm_year < local.tm_year)
        local.tm_yday = utc.tm_yday + 1;
    if (utc.tm_yday == local.tm_yday)
        return (utc.tm_hour - local.tm_hour);
    else if (utc.tm_yday > local.tm_yday)
        return (utc.tm_hour + (24 - local.tm_hour));
    else
        return (-(local.tm_hour + (24 - utc.tm_hour)));
    }






unsigned long timestamp::calibrate_rdtsc_ticks_over_usec_period(unsigned long period_in_usecs)
    {
    unsigned long start_val, end_val;
    struct timeval st, et;
    unsigned long diff = 0;

    gettimeofday(&st, NULL);
    start_val = timestamp::rdtsc64_nosync();
    while (diff < period_in_usecs)
        {
        gettimeofday(&et, NULL);
        diff = (et.tv_sec * 1000000L + et.tv_usec) - (st.tv_sec * 1000000L + st.tv_usec);
        }
    end_val = timestamp::rdtsc64_nosync();
    return ((end_val - start_val) / diff);
    }


