#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <h/latency.hpp>



latency::latency()
    {
    reset_counters();
    display_range = 50;
    return;
    }




latency::~latency()
    {
    return;
    }






void latency::calc(const string &file, bool reset_counter)
    {
    FILE *fp;

    if ((fp = fopen(file.c_str(), "w")) != NULL)
        {
        calc(fp, reset_counter);
        fclose(fp);
        }
    return;
    }




void latency::calc(FILE *fp, bool reset_counter)
    {
    double f = 0.0, cf = 0.0;
    long range_total = 0, running_total = 0;
    int range_len;

    for (int i = 0; i < million; i += display_range)
        {
        range_total = 0;
        range_len = ((million - i) > display_range) ? display_range : (million - i);
        for (int y = 0; y < range_len; y++)
            range_total += counters[i + y];
        if (range_total > 0)
            {
            running_total += range_total;
            f = (double)range_total / total_count * 100.0;
            cf = (double)running_total / total_count * 100.0;
            fprintf(fp, "%6d - %6d  count %10ld    f %9.5f   cf %9.5f\n", i, i + range_len - 1, range_total, f, cf);
            }
        }
    fprintf(fp, "total count=%ld  under=%ld(%.4f%%)  over=%ld(%.4f%%)  av time %ld\n\n\n", total_count, under, ((double)under / total_count) * 100.0, over, ((double)over / total_count) * 100.0, total_time / total_count);
    if (reset_counter == true)
        reset_counters();
    return;
    }





void latency::calc(char *buf, int &len, bool reset_counter)
    {
    double f = 0.0, cf = 0.0;
    long range_total = 0, running_total = 0;
    int range_len;

    len = 0;
    for (int i = 0; i < million; i += display_range)
        {
        range_total = 0;
        range_len = ((million - i) > display_range) ? display_range : (million - i);
        for (int y = 0; y < range_len; y++)
            range_total += counters[i + y];
        if (range_total > 0)
            {
            running_total += range_total;
            f = (double)range_total / total_count * 100.0;
            cf = (double)running_total / total_count * 100.0;
            len += sprintf(buf + len, "%6d - %6d  count %10ld    f %9.5f   cf %9.5f\n", i, i + range_len - 1, range_total, f, cf);
            }
        }
    len += sprintf(buf + len, "total count=%ld  under=%ld(%.4f%%)  over=%ld(%.4f%%)\n\n\n", total_count, under, ((double)under / total_count) * 100.0, over, ((double)over / total_count) * 100.0);
    if (reset_counter == true)
        reset_counters();
    return;
    }







ostream &operator<<(ostream &os, const latency &l)
    {
    double f = 0.0, cf = 0.0;
    long range_total = 0, running_total = 0;
    int range_len;
    char buf[4096];

    os << buf;
    for (int i = 0; i < l.million; i += l.display_range)
        {
        range_total = 0;
        range_len = ((l.million - i) > l.display_range) ? l.display_range : (l.million - i);
        for (int y = 0; y < range_len; y++)
            range_total += l.counters[i + y];
        if (range_total > 0)
            {
            running_total += range_total;
            f = (double)range_total / l.total_count * 100.0;
            cf = (double)running_total / l.total_count * 100.0;
            sprintf(buf, "%6d - %6d  count %10ld    f %9.5f   cf %9.5f\n", i, i + range_len - 1, range_total, f, cf);
            os << buf;
            }
        }
    sprintf(buf, "total count=%ld  under=%ld(%.4f%%)  over=%ld(%.4f%%)\n\n\n", l.total_count, l.under, ((double)l.under / l.total_count) * 100.0, l.over, ((double)l.over / l.total_count) * 100.0);
    os << buf;
    return (os);
    }


