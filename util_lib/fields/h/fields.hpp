#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>



using namespace std;




// class allows the caller to create and extract arbitrary field base messages that are variable length.
// all the basic types are supported as well as string, c strings, and buffer, length (not null terminated)
// variable length fields can be 0 to 255 bytes in size as the length is stored in a single byte.
// This is primarily used for the GAIA gui interface to/from the pricer.

class fields
    {
public:
    fields(unsigned int max_len)
        {
        buffer = new unsigned char [max_len];
        max_buffer_len = max_len;
        current_buffer_len = 0;
        current_iterator = 0;
        }
    ~fields()
        {
        delete [] buffer;
        }
    template<typename T> inline void add_field(T value);
    inline void add_field(char *value);
    inline void add_field(const char *value);
    inline void add_field(signed char *value);
    inline void add_field(const signed char *value);
    inline void add_field(unsigned char *value);
    inline void add_field(const unsigned char *value);
    inline void add_field(const char *value, size_t len);
    inline void add_field(const signed char *value, size_t len);
    inline void add_field(const unsigned char *value, size_t len);
    inline void add_field(const std::string &value);
    inline void add_field_fixed(const std::string &value, size_t len);
    inline void add_field_fixed_length(const std::string &value, size_t len, char pad);
    inline void insert(const char *src, const size_t len);
    inline void copy(char *dst, const size_t max_len, size_t &len);
    inline size_t size();
    inline const unsigned char *ptr();
    void dump(FILE *fp);
    inline void clear();
    inline void get_reset();
    template <typename T> inline void get_field(T &value);
    inline void get_field(char *value);
    inline void get_field(signed char *value);
    inline void get_field(unsigned char *value);
    inline void get_field(char *value, size_t &len);
    inline void get_field(signed char *value, size_t &len);
    inline void get_field(unsigned char *value, size_t &len);
    inline void get_field(std::string &value);
    inline void get_field_fixed(std::string &value, size_t len);
    friend ostream &operator<<(ostream &os, const fields &l);
private:
    fields(const fields &);
    fields &operator=(const fields &);
    static const char               *mc_no_space;
    static const char               *mc_data_exhausted;
    static const char               *mc_field_too_big;
    static const char               *mc_insert_too_big;
    static const char               *mc_copy_too_big;
    inline void copy_data_in(const void *src, size_t len);
    inline void copy_data_in_fixed(const void *src, size_t len);
    unsigned int                    max_buffer_len;
    unsigned int                    current_buffer_len;
    unsigned int                    current_iterator;
    unsigned char                   *buffer;
    };




// for all fixed length basic types

template<typename T> inline void fields::add_field(T value)
    {
    if (current_buffer_len + sizeof(T) > max_buffer_len)
        throw std::logic_error(mc_no_space);
    *((T *)(buffer + current_buffer_len)) = value;
    current_buffer_len += sizeof(value);
    return;
    }


// for c strings null terminated

inline void fields::add_field(char *value)
    {
    copy_data_in(value, strlen(value));
    return;
    }

inline void fields::add_field(signed char *value)
    {
    copy_data_in(value, strlen((const char *)value));
    return;
    }

inline void fields::add_field(unsigned char *value)
    {
    copy_data_in(value, strlen((const char *)value));
    return;
    }

inline void fields::add_field(const char *value)
    {
    copy_data_in(value, strlen(value));
    return;
    }

inline void fields::add_field(const signed char *value)
    {
    copy_data_in(value, strlen((const char *)value));
    return;
    }

inline void fields::add_field(const unsigned char *value)
    {
    copy_data_in(value, strlen((const char *)value));
    return;
    }



// for arbitrary memory, not null terminated

inline void fields::add_field(const char *value, size_t len)
    {
    copy_data_in(value, len);
    return;
    }

inline void fields::add_field(const signed char *value, size_t len)
    {
    copy_data_in(value, len);
    return;
    }

inline void fields::add_field(const unsigned char *value, size_t len)
    {
    copy_data_in(value, len);
    return;
    }


// for c++ strings, can be extracted as any variable type field

inline void fields::add_field(const std::string &value)
    {
    copy_data_in(value.c_str(), value.size());
    return;
    }

inline void fields::add_field_fixed(const std::string &value, size_t len)
    {
    copy_data_in_fixed(value.c_str(), value.size());
    return;
    }
inline void fields::add_field_fixed_length(const std::string &value, size_t len, char pad)
    {
    if (value.size () >= len)
        {
        copy_data_in_fixed(value.c_str(), len);
        }
    else
        {
        int to_pad = len - value.size();
        copy_data_in_fixed(value.c_str(), value.size());
        for (int i = 0 ; i < to_pad ; ++i)
            {
            copy_data_in_fixed(&pad, 1);
            }
        }

    return;
    }





// for all fixed length basic types

template <typename T> inline void fields::get_field(T &value)
    {
    if (current_iterator + sizeof(T) > current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    value = *((T *)(buffer + current_iterator));
    current_iterator += sizeof(value);
    return;
    }


// for c strings null terminated

inline void fields::get_field(char *value)
    {
    get_field((unsigned char *)value);
    return;
    }

inline void fields::get_field(signed char *value)
    {
    get_field((unsigned char *)value);
    return;
    }

inline void fields::get_field(unsigned char *value)
    {
    size_t len;

    if (current_iterator >= current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    len = *(buffer + current_iterator++);
    if (current_iterator + len > current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    memcpy(value, buffer + current_iterator, len);
    *(value + len) = 0;
    current_iterator += len;
    return;
    }




// for arbitrary memory, not null terminated

inline void fields::get_field(char *value, size_t &len)
    {
    get_field((unsigned char *)value, len);
    return;
    }

inline void fields::get_field(signed char *value, size_t &len)
    {
    get_field((unsigned char *)value, len);
    return;
    }

inline void fields::get_field(unsigned char *value, size_t &len)
    {
    if (current_iterator >= current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    len = *(buffer + current_iterator++);
    if (current_iterator + len > current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    memcpy(value, buffer + current_iterator, len);
    current_iterator += len;
    return;
    }


// for c++ strings

inline void fields::get_field(std::string &value)
    {
    size_t len;

    if (current_iterator >= current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    len = *(buffer + current_iterator++);
    if (current_iterator + len > current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    value.assign((char *)(buffer + current_iterator), len);
    current_iterator += len;
    return;
    }

inline void fields::get_field_fixed(std::string &value, size_t len)
    {
    if (current_iterator >= current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    if (current_iterator + len > current_buffer_len)
        throw std::logic_error(mc_data_exhausted);
    value.assign((char *)(buffer + current_iterator), len);
    current_iterator += len;
    return;
    }




// resets the reading of the data back to the first field

inline void fields::get_reset()
    {
    current_iterator = 0;
    return;
    }



// clears all fields

inline void fields::clear()
    {
    current_buffer_len = 0;
    get_reset();
    return;
    }



inline size_t fields::size()
    {
    return (current_buffer_len);
    }



inline const unsigned char *fields::ptr()
    {
    return (buffer);
    }



inline void fields::insert(const char *src, const size_t len)
    {
    clear();
    if (len > max_buffer_len)
        throw std::logic_error(mc_insert_too_big);
    memcpy(buffer, src, len);
    current_buffer_len = len;
    return;
    }



inline void fields::copy(char *dst, size_t const max_len, size_t &len)
    {
    if (max_len < current_buffer_len)
        throw std::logic_error(mc_copy_too_big);
    memcpy(dst, buffer, current_buffer_len);
    len = current_buffer_len;
    return;
    }






// generic routine to copy variable length fields into memory

inline void fields::copy_data_in(const void *src, size_t len)
    {
    if (len > 255)
        throw std::logic_error(mc_field_too_big);
    if (current_buffer_len + len + sizeof(char) > max_buffer_len)
        throw std::logic_error(mc_no_space);
    *(buffer + current_buffer_len) = (unsigned char)len;
    ++current_buffer_len;
    memcpy(buffer + current_buffer_len, src, len);
    current_buffer_len += len;
    return;
    }

inline void fields::copy_data_in_fixed(const void *src, size_t len)
    {
    if (len > 255)
        throw std::logic_error(mc_field_too_big);
    if (current_buffer_len + len > max_buffer_len)
        throw std::logic_error(mc_no_space);
    memcpy(buffer + current_buffer_len, src, len);
    current_buffer_len += len;
    return;
    }






