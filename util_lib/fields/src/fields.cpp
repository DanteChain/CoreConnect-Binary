#include <pthread.h>
#include <cstdio>
#include <iostream>
#include <h/fields.hpp>




using namespace std;



const char *fields::mc_no_space = "no space left";
const char *fields::mc_data_exhausted = "data exhausted";
const char *fields::mc_field_too_big = "field too big";
const char *fields::mc_insert_too_big = "insert too big";
const char *fields::mc_copy_too_big = "copy too big";





// debug dump  of the stored fields in hex

void fields::dump(FILE *fp)
    {
    fprintf(fp, "field data dump, max buffer len:%d  current buffer len:%d read iterator:%d\n", max_buffer_len, current_buffer_len, current_iterator);
    if (current_buffer_len > 0)
        {
        fprintf(fp, "    ");
        for (unsigned int i = 0; i < current_buffer_len; i++)
            fprintf(fp, "%02X ", buffer[i]);
        fprintf(fp, "\n");
        }
    return;
    }




// debug dump  of the stored fields in hex

ostream &operator<<(ostream &os, const fields &l)
    {
    os << "field data dump, max buffer len:" << l.max_buffer_len << "  current buffer len:" << l.current_buffer_len << " read iterator:" << l.current_iterator << endl;
    if (l.current_buffer_len > 0)
        {
        os << "    ";
        os << hex << setfill('0') << uppercase;
        for (unsigned int i = 0; i < l.current_buffer_len; i++)
            os << setw(2) << (unsigned int)l.buffer[i] << " ";
        os << endl;
        os.copyfmt(std::ios(NULL));
        }
    return (os);
    }



