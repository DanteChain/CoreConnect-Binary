#pragma once

#include <string>
#include <netinet/in.h>

// Frame is: <2 byte binary frame len> <variable len data>
// length is total length of frame not including length bytes.
// so min frame length is 2, max frame length is 65537
// min length of a body is 0, max len of a body is 65535

// The generic framer can process and parse N individual streams in parallel. The number of supported streams is supplied in the constructor.
// When a parsed frame is identified the callback parameters include the stream id.
// The stream ID is a number from 0 - N - 1, where N is the number of streams supplied in the constructor.

class generic_framer1
    {
public:
    generic_framer1(unsigned int number_of_streams, unsigned short max_rx_frame_size);
    virtual ~generic_framer1();
    enum framer_errors
        {
        E_frmr_no_error = 0,
        E_frmr_hdr_len_range = 5000,
        E_frmr_stream_range = 5001,
        E_frmr_body_len_range = 5002,
        E_frmr_bridge_hdr_len_range = 5003,
        E_frmr_bridge_body_len_range = 5004,
        };
    int parse_raw_rx_data(unsigned int stream, const char *src, unsigned int len);
    inline int clear(unsigned int stream);
    virtual int process_rx_frame(unsigned int stream, const char *src, unsigned int len) = 0;
    static const unsigned short sm_min_frame_length = 2U;
    static const unsigned short sm_max_frame_length = 65535U;
    static const unsigned short sm_max_number_streams = 100;
private:
    generic_framer1(const generic_framer1 &) = delete;
    generic_framer1 &operator=(const generic_framer1 &) = delete;
    inline void clear_stream(unsigned int stream);
    enum frame_state
        {
        frame_state_hdr1,
        frame_state_hdr2,
        frame_state_body,
        };
    typedef struct
        {
        unsigned short              m_max_rx_frame_length;
        enum frame_state            m_rx_frame_state;
        unsigned short              m_rx_len;
        unsigned short              m_required_len;
        char                        *m_rx_buf;
        } STREAM_CONTROL;
    STREAM_CONTROL                  *m_sc;
    unsigned int                    m_number_of_streams;
    };




// stream validated public clear() function.

inline int generic_framer1::clear(unsigned int stream)
    {
    if (stream >= m_number_of_streams)
        return (E_frmr_stream_range);
    clear_stream(stream);
    return (E_frmr_no_error);
    }





// private clear() function, stream assumed to have already been validated

inline void generic_framer1::clear_stream(unsigned int stream)
    {
    m_sc[stream].m_rx_frame_state = frame_state_hdr1;
    return;
    }



