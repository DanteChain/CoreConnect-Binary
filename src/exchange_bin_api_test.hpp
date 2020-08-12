#pragma once


#include <string>
#include <list>
#include <set>
#include <time/h/datestamp.hpp>
#include <time/h/timestamp.hpp>
#include <logger/h/logger.hpp>
#include <event_engine2/h/ev_manager.hpp>
#include <generic_framer/h/generic_framer1.hpp>
#include <tcp/h/tcp_client.hpp>



using namespace std;





class exchange_bin_api_test : public generic_framer1, public ev_manager
    {
public:
    template<class T>
    void prompt(string prompt,T &input, T default_val)
    {
        cout <<prompt <<" ("<<default_val<<")"<<endl;
        cin >> input;
        if(input == 0)
        {
            input = default_val;
        } 
        cout << input <<endl;

    }
    string m_config_file;
    int process_new_order(char *buf);
    // int prompt_modify_order();
    // int prompt_cancel_order();
    // int prompt_cancel_all_orders();
    int prompt_modify_leverage(char * buf);
    int prompt_working_orders_request();
    int prompt_fills_request();
    int prompt_asset_positions();
    int process_symbols();
    int prompt_depth();
    
    

    int process_new_order(NEW_ORDER_REQUEST&orderrequest);
    // int process_modify_order(MODIFY_ORDER_REQUEST&orderrequest);
    // int process_cancel_order(CANCEL_ORDER_REQUEST&orderrequest);
    // int process_cancel_all_orders(CANCEL_ALL_ORDERS_REQUEST&orderrequest);
    int process_modify_leverage(MODIFY_LEVERAGE_REQUEST&levrequest);
    int process_working_orders_request(WORKING_ORDERS_REQUEST&workingOrderRequest);
    int process_fills_request(FILL_REQUEST&workingOrderRequest);
    exchange_bin_api_test(const std::string &name, const std::string &gateway_client_id, const std::string password, const std::string omgip, const std::string omgport, const std::string mdgip, const std::string mdgport, const std::string root_filename);
    ~exchange_bin_api_test();
    int run();
    int initialisation(int ac, char **av);
    inline void send_input(char *buf);
    enum states
        {
        task_initialise,
        };
    enum events
        {
        ev_startup = 0,
        ev_idle_tmr = 1,
        ev_load_tmr = 2,
        ev_tcp_accept = 7,
        ev_tcp_read = 8,
        ev_tcp_exception = 9,
        ev_console_read = 10,
        ev_write_latency_file = 12,
        ev_console_input = 13,
        ev_tcp_read_mdg = 14,
        ev_tcp_exception_mdg = 8999,
        };
    enum timers
        {
        tmr_idle,

        };
    void write_bulk_new_orders(BINARY_FRAME f, int no_loops);
private:
    //
    

    exchange_bin_api_test(const exchange_bin_api_test &);
    exchange_bin_api_test &operator=(const exchange_bin_api_test &);
    virtual int ev_app_event_callback(ev_event *pev);
    virtual int ev_system_event_callback(int event);
    virtual int process_rx_frame(unsigned int index, const char *src, unsigned int len);
    int fsm_initialise(ev_event *pev);
    int initialise_task();
    int tcp_exception();
    int tcp_write(char *src, int len);
    int tcp_read();
    int tcp_read_mdg();
    int send_heartbeat();
    int process_idle_tmr();
    int process_load_tmr();
    int console_read(ev_event *pev);
    int p_set_flags(int fd, int flags);
    int write_latency_file();
    int process_a_logon();
    int process_a_logoff(char *buf);
    int process_a_new_order(char *buf);
    int process_a_modify_order(char *buf);
    int process_a_cancel_order(char *buf);
    int process_a_cancel_all_orders(char *buf);
    int process_a_cancel_replace(char *buf);
    int process_working_orders(char *buf);
    int process_get_fills(char *buf);
    int process_new_order_single_response(NEW_ORDER_RESPONSE &m);
    int process_modify_order_response(MODIFY_ORDER_RESPONSE &m);
    int process_cancel_order_response(CANCEL_ORDER_RESPONSE &m);
    int process_cancel_all_orders_response(CANCEL_ALL_ORDERS_RESPONSE &m);
    //int process_cancel_replace_order_response(CANCEL_REPLACE_ORDER_RESPONSE &m);
    int process_working_orders_response(WORKING_ORDERS_RESPONSE &m);
    int process_fill_response(FILL_RESPONSE &m);
    int process_working_order_notification(WORKING_ORDER_NOTIFICATION &m);
    int process_fill_notification(FILL_NOTIFICATION &m);
    int process_modify_order_notification(MODIFY_ORDER_NOTIFICATION &m);
    int process_cancel_order_notification(CANCEL_ORDER_NOTIFICATION &m);
    int process_session_notification(SESSION_NOTIFICATION &m);
    //int process_stop_trigger_notification(STOP_TRIGGER_NOTIFICATION &m);
    int process_gateway_client_logoff_notification(GATEWAY_CLIENT_LOGOFF_NOTIFICATION &m);
    int process_a_new_order_bulk(char *buf);
    int process_json_structures(char *buf);
    int process_users_of_gatewayclients(char *buf);
    int process_add_instrument(char * buf);
    int process_load_settings_tmr(int timerType);
    int process_set_load_setting(char *buf);
 int process_random_no();
    
    int print_menu();
    int do_latency();
    int check_set();
    int m_nRandomNumber;
    std::string                     m_gateway_client_id;
    std::string                     m_name;
    std::string                     m_password;
    std::string                     m_ip;
    std::string                     m_port;
    std::string                     m_mdg_ip;
    std::string                     m_mdg_port;
    std::string                     m_log_filename;
    std::string                     m_latency_filename;
    int                             m_main_task_id;
    int                             m_state;
    logger                          *m_log;
    int                             (exchange_bin_api_test::*m_pfunc[1])(ev_event *pev);
    bool                            m_event_delete;
    tcp_client                      m_tcpc;
    tcp_client                      m_mdg_tcpc;
    tcp_connection                  *m_con;
    tcp_connection                  *m_mdg_con;
    int                             m_tcp_rx_idle_count;
    int                             m_tcp_tx_idle_count;
    bool                            m_tcp_connected;
    latency                         m_latency;
    int                             m_rdtsc_ticks_per_usec;
    int                             m_idle_count;
    int                             m_NumberToSend;
    int                             m_NumberInBatch;
    int                             m_TmrIntervalMsecs;
    int                             m_NumberUsers;
    int                             m_StartUserID;
    unsigned long                   m_UserTag;
    set<unsigned long>              m_user_tags;
    int                             m_counter;
    };



class console_input : public ev_event
    {
public:
    console_input(char *arg_buf) : ev_event(exchange_bin_api_test::ev_console_input)
        {
        strcpy(buf, arg_buf);
        }
    char buf[512];
    };




inline void exchange_bin_api_test::send_input(char *buf)
    {
    ev_tx_id(m_main_task_id, new console_input(buf));
    return;
    }



inline string to_string(char *src, int len)
    {
    char tmp[1024];

    strncpy(tmp, src, len);
    tmp[len] = 0;
    return (tmp);
    }


inline string to_string(unsigned char *src, int len)
    {
    char tmp[1024];

    strncpy(tmp, (char *)src, len);
    tmp[len] = 0;
    return (tmp);
    }





