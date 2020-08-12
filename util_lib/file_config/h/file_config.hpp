#pragma once

#include <iostream>
#include <string>
#include <map>


enum config_errors
    {
    No_Error = 0,
    E_config_open = 2000,
    E_config_syntax,
    E_config_null_key,
    E_config_eof,
    E_config_read,
    E_config_unknown,
    };





// class stores key/value pairs as read in from a configuration file. The key/value
// pairs are stored within named sections. A config file may contain many sections.
// Only 1 section may be read at a time. Blank lines and lines with just whitespace
// are ignored and any data after a # is treated as a comment. Leading and trailing
// whitespace is removed from all lines. The indentation is just for readability
// purposes only. A section name may not be called [END] for obvious reasons.
// Sample section
//
// [A SECTION NAME]
//      key_name1 = some sort of data
//      key_name2 = some more data
//      key_name1 = 45
//# This line is a comment line
//      key_nameA = host port 45000 # this line has trailing comment.
//[END]
//

class file_config
    {
public:
    typedef std::multimap<std::string, std::string> cfg_key_value;              // typedefs to make code easier
    typedef cfg_key_value::value_type value_type;                               // to read.
    typedef cfg_key_value::key_type key_type;                                   //
    file_config();                                                              // Constructor
    ~file_config();                                                             // Destructor
    int read_file(const std::string &config_file, const std::string &section_name);// read config file contents
    void read_reset();                                                          // reset to start of multimap or key
    bool read_key_value(std::string &key, std::string &value);                  // read a specifc key
    bool find_key(const std::string &key);                                            // read iterating through multimap
    void dump(std::ostream &out);                                               // dump contents to out debugging only
private:
    file_config(const file_config &);                                           // copy constructor, do not implement
    file_config &operator=(const file_config &);                                // assignment op, do not implement
    void clean_line(std::string &line);                                         // clean line up
    int find_section(std::ifstream &file);                                      // find the specified section
    int process_section(std::ifstream &file);                                   // process the lines of a section
    int read_line(std::ifstream &file, std::string &line);                      // read a raw file line and tidy
    cfg_key_value               m_data;                                         // hold key/values pairs, duplicates ok
    std::string                 m_file_name;                                    // config file name
    std::string                 m_section_name;                                 // config section name
    cfg_key_value::iterator     m_pos;                                          // current iterator position
    cfg_key_value::iterator     m_end_pos;                                      // end iterator position
    };
