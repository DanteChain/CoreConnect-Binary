#include <h/generic_framer1.hpp>
#include <cstring>
#include <cstdio>




generic_framer1::generic_framer1(unsigned int number_of_streams, unsigned short max_rx_frame_length)
    {
    m_number_of_streams = (number_of_streams > sm_max_number_streams) ? sm_max_number_streams : number_of_streams;
    m_sc = new STREAM_CONTROL[m_number_of_streams];
    for (unsigned int i = 0; i < m_number_of_streams; i++)
        {
        m_sc[i].m_max_rx_frame_length = (max_rx_frame_length < sm_min_frame_length) ? sm_min_frame_length : max_rx_frame_length;
        m_sc[i].m_rx_frame_state = frame_state_hdr1;
        m_sc[i].m_required_len = 0;
        m_sc[i].m_rx_len = 0;
        m_sc[i].m_rx_buf = new char [m_sc[i].m_max_rx_frame_length];
        };
    }





generic_framer1::~generic_framer1()
    {
    for (unsigned int i = 0; i < m_number_of_streams; i++)
        {
        if (m_sc[i].m_rx_buf != 0)
            delete [] m_sc[i].m_rx_buf;
        m_sc[i].m_rx_buf = 0;
        }
    delete [] m_sc;
    m_sc = 0;
    return;
    }





// FSM parser for input raw data stream. Should follow the format <unsigned short frame len> <variable length data, can be 0>

int generic_framer1::parse_raw_rx_data(unsigned int stream, const char *src, unsigned int len)
    {
    STREAM_CONTROL *sc;
    int ret = E_frmr_no_error;
    unsigned int clen;

    if (stream >= m_number_of_streams)
        return (E_frmr_stream_range);
    sc = &m_sc[stream];
    while (len && ret == E_frmr_no_error)
        {
        switch (sc->m_rx_frame_state)
            {
        case frame_state_hdr1:
            sc->m_rx_len = 1;
            sc->m_rx_frame_state = frame_state_hdr2;
            sc->m_rx_buf[0] = *src++;
            --len;
            break;
        case frame_state_hdr2:
            ++sc->m_rx_len;
            sc->m_rx_frame_state = frame_state_body;
            sc->m_rx_buf[1] = *src++;
            --len;
            sc->m_required_len = *(unsigned short *)sc->m_rx_buf;
            if (sc->m_required_len < sizeof(unsigned short))
                {
                ret = E_frmr_hdr_len_range;
                break;
                }
        case frame_state_body:
            clen = (len > sc->m_required_len) ? sc->m_required_len : len;
            if (sc->m_rx_len + clen > sc->m_max_rx_frame_length)
                ret = E_frmr_body_len_range;
            else
                {
                memcpy(sc->m_rx_buf + sc->m_rx_len, src, clen);
                sc->m_rx_len += clen;
                src += clen;
                len -= clen;
                if ((sc->m_required_len -= clen) == 0)
                    {
                    ret = process_rx_frame(stream, sc->m_rx_buf, sc->m_rx_len);
                    sc->m_rx_frame_state = frame_state_hdr1;
                    }
                }
            break;
            }
        }
    if (ret != E_frmr_no_error)
        clear_stream(stream);
    return (ret);
    }

