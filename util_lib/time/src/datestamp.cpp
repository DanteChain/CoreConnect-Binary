#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <h/datestamp.hpp>





datestamp::datestamp()
    {
    }




int datestamp::get_current_skew()
    {
    time_t temp = time(0);

    struct tm *tms = localtime(&temp);
    return (0 - tms->tm_gmtoff);
    }




std::string datestamp::get_yymmdd_as_string(int skew)
    {
    std::string s;

    get_yymmdd(s, skew);
    return (s);
    }




std::string datestamp::get_short_date_as_string(int skew)
    {
    std::string d = get_yymmdd_as_string(skew);
    char result[4];

    result[3] = '\0';
    int day = (d[4] - '0') * 10 + d[5] - '0';
    if (day < 26)
        result[2] = day + 'A';
    else if ( day < 52 )
        result[2] = day - 26 + 'a';
    else
        result[2] = day - 52 + '0';
    result[1] = (d[2] - '0') * 10 + d[3] - '0' + 'A';
    result[0] = (d[0] - '0') * 10 + d[1] - '0' + 'A';
    return std::string(result);
    }





void datestamp::get_yyyymmdd(std::string &dst, int skew)
    {
    char buf[128];
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    sprintf(buf, "%4.4d%2.2d%2.2d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    dst = buf;
    return;
    }




void datestamp::get_mmddyyyy(std::string &dst, int skew)
    {
    char buf[128];
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    sprintf(buf, "%2.2d%2.2d%4.4d", t.tm_mon + 1, t.tm_mday, t.tm_year + 1900);
    dst = buf;
    return;
    }





void datestamp::get_yymmdd(std::string &dst, int skew)
    {
    char buf[128];
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    sprintf(buf, "%2.2d%2.2d%2.2d", t.tm_year - 100, t.tm_mon + 1, t.tm_mday);
    dst = buf;
    return;
    }




void datestamp::get_yymmdd(unsigned int &dst, int skew)
    {
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    dst = (t.tm_year - 100) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
    }





std::string datestamp::get_yymmdd(int skew)
    {
    char buf[32];
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    sprintf(buf, "%2.2d%2.2d%2.2d", t.tm_year - 100, t.tm_mon + 1, t.tm_mday);
    return (std::string(buf));
    }





void datestamp::get_yyyymmdd(unsigned int &dst, int skew)
    {
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    dst = (t.tm_year + 1900) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
    }




std::string datestamp::get_yyyymmdd(int skew)
    {
    char buf[32];
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    sprintf(buf, "%4.4d%2.2d%2.2d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    return (std::string(buf));
    }




std::string datestamp::get_mmddyyyy(int skew)
    {
    char buf[32];
    struct timeval tmp;
    struct tm t;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec += skew;
    localtime_r(&tmp.tv_sec, &t);
    sprintf(buf, "%2.2d%2.2d%4.4d", t.tm_mon + 1, t.tm_mday, t.tm_year + 1900);
    return (std::string(buf));
    }


