#pragma once

#include <string>


#include <ctime>
#include <sys/time.h>



class datestamp
    {
public:
    datestamp();
    static void get_yyyymmdd(std::string &dst, int skew = 0);
    static void get_mmddyyyy(std::string &dst, int skew = 0);
    static void get_yyyymmdd(unsigned int &dst, int skew = 0);
    static void get_yymmdd(std::string& dst, int skew = 0);
    static void get_yymmdd(unsigned int &dst, int skew = 0);
    static std::string get_yymmdd(int skew = 0);
    static std::string get_yyyymmdd(int skew = 0);
    static std::string get_mmddyyyy(int skew = 0);
    static std::string get_yymmdd_as_string(int skew = 0);
    static std::string get_short_date_as_string(int skew = 0);
    static int get_current_skew();
private:
    datestamp(const datestamp &);
    datestamp &operator=(const datestamp &);
    };

