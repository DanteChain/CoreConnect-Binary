#pragma once


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>



using namespace std;





class latency
    {
public:
    latency();
    ~latency();
    inline void add(int usec);
    inline void reset_counters();
    inline void set_display_range(int range);
    inline long size();
    void calc(const string &file, bool reset_counter);
    void calc(char *buf, int &len, bool reset_counter = false);
    void calc(FILE *fp, bool reset_counter = false);
    friend ostream &operator<<(ostream &os, const latency &l);
private:
    latency(const latency &) = delete;
    latency &operator=(const latency &) = delete;
    static const int        million = 1000000;
    long                    counters[million];
    long                    under;
    long                    over;
    long                    total_count;
    long                    total_time;
    int                     display_range;
    };



inline long latency::size()
    {
    return (total_count);
    }



inline void latency::add(int usec)
    {
    ++total_count;
    total_time += usec;
    if (usec >= million)
        ++over;
    else if (usec < 0)
        ++under;
    else
        ++counters[usec];
    return;
    }






inline void latency::reset_counters()
    {
    under = 0;
    over = 0;
    total_count = 0;
    total_time = 0;
    memset(counters, 0, sizeof(counters));
    return;
    }



inline void latency::set_display_range(int range)
    {
    display_range = (range < 0 || range > 100000) ? 100 : range;
    return;
    }

