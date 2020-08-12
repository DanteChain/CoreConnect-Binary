#pragma once


#include <map>
#include <string>

    enum tcp_errors
    {
        E_tcp_no_error = 0,
        E_tcp_socket = 9000,
        E_tcp_bind_failed = 9001,
        E_tcp_listen_fail = 9002,
        E_tcp_accept_failed = 9003,
        E_tcp_listen_in_progress = 9004,
        E_tcp_already_connected = 9005,
        E_tcp_connect_failed = 9006,
        E_tcp_select_failed = 9007,
        E_tcp_connect_timeout = 9008,
        E_tcp_connect_error = 9009,
        E_tcp_conn_in_progress = 9010,
        E_tcp_already_closed = 9011,
        E_tcp_close_fail = 9012,
        E_tcp_null_ptr = 9013,
        E_tcp_max_read_len = 9014,
        E_tcp_read_fail = 9015,
        E_tcp_read_error = 9016,
        E_tcp_max_write_len = 9017,
        E_tcp_would_block = 9018,
        E_tcp_write_fail = 9019,
        E_tcp_get_flags = 9020,
        E_tcp_clear_flags = 9021,
        E_tcp_set_flags = 9022,
        E_tcp_set_sock_reuse = 9023,
        E_tcp_set_sock_linger = 9024,
        E_tcp_set_sock_kalive = 9025,
        E_tcp_set_disable_nagle = 9026,
        E_tcp_set_sock_tx_buf = 9027,
        E_tcp_set_sock_rx_buf = 9028,
        E_tcp_invalid_host_name = 9029,
        E_tcp_host_not_defined = 9030,
        E_tcp_invalid_service_name = 9031,
        E_tcp_invalid_service_port = 9032,
        E_tcp_service_not_defined = 9033,
        E_tcp_read_buffer_sizes = 9034,
        E_tcp_set_disable_cork = 9035,
    };

    /**
     * Create an instance of this class to convert error codes to strings. A single static
     * instance will suffice.
     */
    class TCPErrorStrings
    {
        class Initialiser
        {
        public:
            Initialiser()
            {
                m_MessageText[E_tcp_socket] = "socket";
                m_MessageText[E_tcp_bind_failed] = "bind_failed";
                m_MessageText[E_tcp_listen_fail] = "listen_fail";
                m_MessageText[E_tcp_accept_failed] = "accept_failed";
                m_MessageText[E_tcp_listen_in_progress] = "listen_in_progress";
                m_MessageText[E_tcp_already_connected] = "already_connected";
                m_MessageText[E_tcp_connect_failed] = "connect_failed";
                m_MessageText[E_tcp_select_failed] = "select_failed";
                m_MessageText[E_tcp_connect_timeout] = "connect_timeout";
                m_MessageText[E_tcp_connect_error] = "connect_error";
                m_MessageText[E_tcp_conn_in_progress] = "conn_in_progress";
                m_MessageText[E_tcp_already_closed] = "already_closed";
                m_MessageText[E_tcp_close_fail] = "close_fail";
                m_MessageText[E_tcp_null_ptr] = "null_ptr";
                m_MessageText[E_tcp_max_read_len] = "max_read_len";
                m_MessageText[E_tcp_read_fail] = "read_fail";
                m_MessageText[E_tcp_read_error] = "read_error";
                m_MessageText[E_tcp_max_write_len] = "max_write_len";
                m_MessageText[E_tcp_would_block] = "would_block";
                m_MessageText[E_tcp_write_fail] = "write_fail";
                m_MessageText[E_tcp_get_flags] = "get_flags";
                m_MessageText[E_tcp_clear_flags] = "clear_flags";
                m_MessageText[E_tcp_set_flags] = "set_flags";
                m_MessageText[E_tcp_set_sock_reuse] = "set_sock_reuse";
                m_MessageText[E_tcp_set_sock_linger] = "set_sock_linger";
                m_MessageText[E_tcp_set_sock_kalive] = "set_sock_kalive";
                m_MessageText[E_tcp_set_disable_nagle] = "set_disable_nagle";
                m_MessageText[E_tcp_set_sock_tx_buf] = "set_sock_tx_buf";
                m_MessageText[E_tcp_set_sock_rx_buf] = "set_sock_rx_buf";
                m_MessageText[E_tcp_invalid_host_name] = "invalid_host_name";
                m_MessageText[E_tcp_host_not_defined] = "host_not_defined";
                m_MessageText[E_tcp_invalid_service_name] = "invalid_service_name";
                m_MessageText[E_tcp_invalid_service_port] = "invalid_service_port";
                m_MessageText[E_tcp_service_not_defined] = "service_not_defined";
                m_MessageText[E_tcp_read_buffer_sizes] = "read_buffer_sizes";
            }
        };

        static std::map<tcp_errors, std::string> m_MessageText;
        static Initialiser m_Initialiser;


    public :
        static std::string GetString (tcp_errors error)
        {
            auto it = m_MessageText.find (error);

            return (it != m_MessageText.end ()) ? (*it).second : "Unknown";
        }
    };

