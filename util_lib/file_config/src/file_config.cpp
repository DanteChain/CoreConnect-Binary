#include <pthread.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <h/file_config.hpp>



using namespace std;



// function is the constructor for the configuration class.
// It just sets up the iterator start and end variables. Obviously
// at this point the multimap has nothing in it so the start and
// end iterators will be the same. (hopefully).

file_config::file_config()
    {
    read_reset();
    return;
    }




// function is the destructor for the configuration class.
// at present it does nothing apart from clear the multimap out.

file_config::~file_config()
    {
    m_data.clear();
    return;
    }





// function is really for internal use only. It dumps the current contents of
// the configuration key values pairs to the supplied ostream. It is really
// only used during development and for debuging.

void file_config::dump(ostream &out)
    {
    cfg_key_value::iterator pos;

    out << "Dumping configuration information from File: " << m_file_name << ",  Section: " << m_section_name << endl;
    for (pos = m_data.begin(); pos != m_data.end(); ++pos)
        out << "    Key:  <" << pos->first << ">,  Value:  <" << pos->second << ">" << endl;
    return;
    }





// function specifies the filename and section to read as arguments. These
// are saved internally only for reference purposes when using the dump()
// member function. Before the file is read any previous help key/value pairs
// are removed from the multimap, then file configuration file is read and the
// specified section processed. If found, the section is read for <key> = <value>
// pairs which are then loaded into the internal multimap. Duplicates are legal.
// Finally the start and end iterators are set to their respective positions ready
// for the data to be read.
// Any error will be returned to the caller and will cause any stored key/value pairs
// to be deleted.

int file_config::read_file(const string &config_file, const string &section_name)
    {
    ifstream file;
    int ret;

    m_file_name = config_file;
    m_section_name = section_name;
    m_data.clear();
    file.open(m_file_name.c_str());
    if (!file)
        ret = E_config_open;
    else if ((ret = find_section(file)) == No_Error)
        ret = process_section(file);
    if (ret != No_Error)
        m_data.clear();
    read_reset();
    return (ret);
    }




// function just reset the start and end iterators ready for future processing.
// If the multimap is empty, then the start and end iterators will be equal.

void file_config::read_reset()
    {
    m_pos = m_data.begin();
    m_end_pos = m_data.end();
    return;
    }





// function reads the next key/value pair from the multimap. If a find key
// has not been specified this function just traverses the entire multimap
// returning each key/value in turn. However if a find_key() has been
// previously specified successfully, then each call will only return
// values for the selected key. If the key is duplicated the key/values
// will be iterated through until exhausted.
// The function returns true if a key/value pair was found or false if
// it wasnt, or at the end of the multimap.

bool file_config::read_key_value(string &key, string &value)
    {
    if (m_pos != m_end_pos)
        {
        key = m_pos->first;
        value = m_pos->second;
        ++m_pos;
        return (true);
        }
    return (false);
    }





// function sets up the object only to select the specified key.
// If the key is found, then repeated calls to read_key_value() will
// return all values for that key. If the key is not found then all
// future calls to read_key_value() will return false until a
// read_reset() is invoked or another successful call to find_key()
// is made.
// The function returns true if the key was found or false if it wasnt.

bool file_config::find_key(const string &key)
    {
    m_pos = m_data.lower_bound(key);
    m_end_pos = m_data.upper_bound(key);
    return (m_pos != m_end_pos);
    }






//**********************************************************************************************
//
//  This section defines all the private member fucntion of the config class.
//
//**********************************************************************************************





// function takes the file data in line and removes the NL char. It then removes any data
// after the comment char #. and then removes trailing and leading whitespace.
// The resulting line is available to the caller to continue to process.

void file_config::clean_line(string &line)
    {
    string::size_type idx;

    if ((idx = line.find('\n')) != string::npos)
        line.resize(idx);
    if ((idx = line.find('#')) != string::npos)
        line.resize(idx);
    if ((idx = line.find_last_not_of("\t ")) == string::npos)
        line.resize(0);
    else if(idx + 1 != line.size())
        line.resize(idx + 1);
    if ((idx = line.find_first_not_of("\t ")) != string::npos && idx > 0)
        line = line.substr(idx);
    return;
    }






// function reads the configuration file looking for the specified section name. If found
// the function returns No_Error, otherwise an error will be returned, usually E_config_eof,
// unless an error has occurred.
// Please Note the tmp.c_str() is used instead of passing the plain string because passing
// the string causes a UMR error in purify which is to be avoided if possible.
// Any error will be returned to the caller.

int file_config::find_section(ifstream &file)
    {
    string line, tmp;
    int ret;

    tmp = '[' + m_section_name + ']';
    while ((ret = read_line(file, line)) == No_Error)
        if (line.find(tmp.c_str()) == 0)
            break;
    return (ret);
    }





// function reads the configuration file reading the lines of data within a section looking
// for key=value pairs. Blank lines are ignored, but all other lines must have a valid
// key = value syntax. Duplicate keys are legal. 0 length keys and values are illegal.
// Once a valid key value pair has been identified it is placed into the multimap for later
// reference.
// Please Note the tmp.c_str() is used instead of passing the plain string because passing
// the string causes a UMR error in purify which is to be avoided if possible.
// Any error will be returned to the caller.

int file_config::process_section(ifstream &file)
    {
    string line, tmp("[END]"), tmp1("[end]"), key, value;
    string::size_type idx;
    int ret = No_Error;

    while (ret == No_Error && (ret = read_line(file, line)) == No_Error)
        {
        if (line.size() == 0)
            ;
        else if (line.find(tmp.c_str()) == 0 || line.find(tmp1.c_str()) == 0)
            break;
        else if ((idx = line.find('=')) == string::npos)
            ret = E_config_syntax;
        else
            {
            key = line.substr(0, idx);
            value = line.substr(idx + 1);
            clean_line(key);
            clean_line(value);
            if (key.size() == 0)
                ret = E_config_null_key;
//            if (value.size() == 0)                                    MDW 20100920 allow zero length values to be valid
//                ret = E_config_null_value;
            else
                m_data.insert(value_type(key, value));
            }
        }
    return (ret);
    }






// function reads a raw line from the configuration file and it then
// proceeds to clean the line removing all comments, leading and trailing
// whitespace and NL chars.
// Any error will be returned to the caller.

int file_config::read_line(ifstream &file, string &line)
    {
    char buf[1024];
    int ret;

    if (file.getline(buf, sizeof(buf)).good())
        {
        line = buf;
        clean_line(line);
        ret = No_Error;
        }
    else if (file.eof())
        ret = E_config_eof;
    else if (file.fail())
        ret = E_config_read;
    else
        ret = E_config_unknown;
    return (ret);
    }

