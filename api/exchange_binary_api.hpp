#pragma once

// Notes
//
// all data is encoded low endian which is the natural byte order on Intel CPU's
// char    8 bit signed binary
// short  16 bit signed binary
// long   64 bit signed binary
// uchar   8 bit unsigned binary
// ushort 16 bit unsigned binary
// ulong  64 bit unsigned binary
//
// This message definition contains NO packing bytes at all. Every field is adjacent to each other
//

#pragma pack(1)

typedef struct
{
    unsigned short msg_len;
} FRAME_HDR;

enum message_types : unsigned short
{
    mt1_gateway_client_logon_request = 0,
    mt1_gateway_client_logoff_request = 1,
    mt1_client_heartbeat = 2,
    mt1_new_order_request = 3,
    mt1_modify_order_request = 4,
    mt1_cancel_order_request = 5,
    mt1_cancel_all_orders_request = 7,
    mt1_cancel_replace_order_request = 8,
    mt1_working_orders_request = 9,
    mt1_fill_request = 10,
    mt1_modify_leverage_request = 11,

    mt1_gateway_client_logon_response = 500,
    mt1_gateway_client_logoff_response = 501,
    mt1_server_heartbeat = 502,
    mt1_new_order_response = 503,
    mt1_modify_order_response = 505,
    mt1_cancel_order_response = 507,
    mt1_cancel_order_notification = 509,
    mt1_fill_notification = 510,
    mt1_session_notification = 511,
    mt1_modify_order_notification = 512,
    mt1_cancel_all_orders_response = 515,
    mt1_cancel_replace_order_response = 517,
    mt1_working_orders_response = 519,
    mt1_stop_trigger_notification = 521,
    mt1_working_order_notification = 522,
    mt1_fill_response = 524,
    mt1_gateway_client_logoff_notification = 527,
    mt1_gateway_client_notification = 528,
    mt1_modify_leverage_response = 529,
    mt1_modify_leverage_notification = 530,


    md_mt_heartbeat = 0,
    md_mt_trade_update = 1,
    md_mt_instrument_state_update = 5,
    md_mt_depth_10_update = 6,

};

enum message_consts
{
    max_len_password = 16,
    len_account_id = 16,
};

typedef struct
{
    unsigned short MessageType;
} HEARTBEAT;

typedef struct
{
    unsigned short MessageType;
    unsigned long GatewayClientID;
    unsigned char Password[max_len_password];
} GATEWAY_CLIENT_LOGON_REQUEST;

typedef struct
{
    unsigned short MessageType;
    unsigned long StatusID;
    unsigned long TimeStamp;
} GATEWAY_CLIENT_LOGON_RESPONSE;

typedef struct
{
    unsigned short MessageType;
} GATEWAY_CLIENT_LOGOFF_REQUEST;

typedef struct
{
    unsigned short MessageType;
    long StatusID;
    unsigned long TimeStamp;
} GATEWAY_CLIENT_LOGOFF_RESPONSE;

typedef struct
{

    unsigned short  MessageType;                 // new order
    unsigned long   UserID;                // specifies a requesting user id
    unsigned long   UserTag;               // specifies a requesting user tag
    
    unsigned long   OrderInstrumentID;           // just some number
    unsigned char   OrderType;                   // order type limit or market
    unsigned char   OrderTimeInForce;            // IOC or DAY
    unsigned char   OrderSide;                   // buy or sell
    unsigned char   OrderExecutionType;          // PostOnly, CloseOnTrigger, ReduceOnly
    unsigned long   OrderQuantity;               // just a number
    unsigned long   OrderDisclosedQuantity;      // just a number
    long            OrderPrice;                           // price of order, fixed decimal (from instrument definitions)
    unsigned short  MarketPriceProtectionTicks; // 0 - 65535
    long            OrderTriggerPrice;                    // trigger price of order, fixed decimal (from instrument definitions)
    unsigned char   TriggerOn;                   // LastTradedPrice, MarkPrice
} NEW_ORDER_REQUEST;

typedef struct
{
    unsigned short MessageType;    // confirm or reject
    unsigned long  UserID;                // specifies a requesting user id
    unsigned long  UserTag;               // specifies a requesting user tag
    unsigned long  ExchangeOrderID; // exchange order id
    unsigned long  ExchangeEventID; // exchange event id
    unsigned long  StatusID;        // rejected
    unsigned long  TimeStamp;
} NEW_ORDER_RESPONSE;


typedef struct
{
    unsigned short MessageType; // modifiy request
    unsigned long UserID;                // specifies a requesting user id
    unsigned long UserTag;               // specifies a requesting user tag
    
    unsigned long ExchangeOrderID;             // exchange order id to modify
    unsigned long OrderQuantityRemaining;      // order quantity remaining
    unsigned char OrderType;                   // order type limit or market
    unsigned char OrderTimeInForce;            // TIF IOC or DAY
    unsigned char OrderExecutionType;          //PostOnly, CloseOnTrigger, ReduceOnly
    unsigned long OrderQuantity;               // quantity to modify
    unsigned long OrderDisclosedQuantity;      // disclosed quantity to modify
    long OrderPrice;                           // price to modify
    unsigned short MarketPriceProtectionTicks; // 0 - 65535
    long OrderTriggerPrice;                    // trigger price to modify
    unsigned char TriggerOn;                   //LastTradedPrice, MarkPrice
} MODIFY_ORDER_REQUEST;
typedef struct
{
    unsigned short MessageType;    // modify order confirm or reject
    unsigned long UserID;                // specifies a requesting user id
    unsigned long UserTag;               // specifies a requesting user tag
    unsigned long ExchangeOrderID; // new exchange order id, may be same or may be a new
    unsigned long ExchangeEventID; // exchange event id
    unsigned long StatusID;        // rejected
    unsigned long TimeStamp;
} MODIFY_ORDER_RESPONSE;

typedef struct
{
    unsigned short MessageType;   // modifiy request
    unsigned long UserID;  // specifies a requesting user id
    unsigned long UserTag; // specifies a requesting user tag
    
    unsigned long InstrumentID;   // specifies unique instrument id
    unsigned long Leverage;       // specifies user leverage
    

} MODIFY_LEVERAGE_REQUEST;

typedef struct
{
    unsigned short MessageType;    // modifiy request
    unsigned long UserID;   // specifies a requesting user id
    unsigned long UserTag;  // specifies a requesting user tag
    unsigned long ExchangeEventID; // exchange event id
    unsigned long StatusID;        // exchange status id
    unsigned long TimeStamp;       // exchange response time
} MODIFY_LEVERAGE_RESPONSE;

typedef struct
{
    unsigned short MessageType;    // modifiy request
                                   // unsigned long UserID;   // specifies a requesting user id
                                   // unsigned long UserTag;  // specifies a requesting user tag
    unsigned long NotificationID;  // specifies unique generated notification id
    unsigned long UserID;          // specifies a unique user id
                                   // unsigned long UserTag;         // user request identifier
    unsigned long InstrumentID;    // specifies unique instrument id
    unsigned long Leverage;        // specifies user leverage
    unsigned long ExchangeEventID; // exchange event id
    unsigned long StatusID;        // exchange status id
    unsigned long TimeStamp;       // exchange response time
} MODIFY_LEVERAGE_NOTIFICATION;











typedef struct
{
    unsigned short MessageType; // cancel order request
    unsigned long UserID;  // specifies a requesting user id
    unsigned long UserTag; // specifies a requesting user tag
    
    unsigned long ExchangeOrderID; // exchange order id to be cancelled
} CANCEL_ORDER_REQUEST;

typedef struct
{
    unsigned short MessageType;    // cancel order confirmed or rejected
    unsigned long UserID;  // specifies a requesting user id
    unsigned long UserTag; // specifies a requesting user tag
    unsigned long ExchangeOrderID; // exchange order id to cancel
    unsigned long ExchangeEventID; // exchange order id
    unsigned long StatusID;        // rejected
    unsigned long TimeStamp;
} CANCEL_ORDER_RESPONSE;

typedef struct
{
    unsigned short MessageType; // cancel order request
    unsigned long UserID;  // specifies a requesting user id
    unsigned long UserTag; // specifies a requesting user tag
    unsigned char cancel_gtc; // 0 = dont cancel GTC 1 = cancel GTC
} CANCEL_ALL_ORDERS_REQUEST;

typedef struct
{
    unsigned short MessageType;        // cancel order confirmed or rejected
    unsigned long UserID;  // specifies a requesting user id
    unsigned long UserTag; // specifies a requesting user tagss
    unsigned long TotalOrdersCanceled; // total number orders successfully canceled
    unsigned long StatusID;            // rejected
    unsigned long TimeStamp;
} CANCEL_ALL_ORDERS_RESPONSE;


















typedef struct
{
    unsigned short MessageType; // cancel order request
    unsigned long UserID;  // specifies a requesting user id
    unsigned long UserTag; // specifies a requesting user tag
    
} WORKING_ORDERS_REQUEST;

typedef struct
{
    unsigned short MessageType;          // cancel order confirmed or rejected
    unsigned long UserID;  // specifies a requesting user id
    unsigned long UserTag; // specifies a requesting user tag
    
    unsigned long NumberOfWorkingOrders; // no of orders still working
    unsigned long StatusID;              // rejected
    unsigned long TimeStamp;
} WORKING_ORDERS_RESPONSE;

typedef struct
{
    unsigned short MessageType; // cancel order request
    unsigned long UserID;       // specifies a unique user id
    unsigned long UserTag;      // user request id
} FILL_REQUEST;

typedef struct
{
    unsigned short MessageType;  // cancel order confirmed or rejected
    unsigned long UserID;        // specifies a unique user id
    unsigned long UserTag;       // user request id
    unsigned long NumberOfFills; // no of fills to be returned
    unsigned long StatusID;      // rejected
    unsigned long TimeStamp;
} FILL_RESPONSE;


typedef struct 
{
    unsigned short MessageType;
    unsigned long UserID;
    unsigned long InstrumentID;
    long OrderSide;
    long OrderType;
    long OrderTimeInForce;
    long OrderExecutionType; //PostOnly, CloseOnTrigger, ReduceOnly
    long Quantity;
    long DisclosedQuantity;
    long Price;
    long MarketPriceProtectionTicks;
    long TriggerPrice;
    long OrderTriggerOn; //LastTradedPrice, MarkPrice
    unsigned long ExchangeOrderID;
    unsigned long ExchangeEventID;
    unsigned long ExchangeStatusID;
    unsigned long ExchangeTimeStamp;
} NEW_ORDER_NOTIFICATION;

typedef struct
{
    unsigned short MessageType;    // unsolicited notifications
    unsigned long UserID;          // specifies a unique user id

    unsigned long ExchangeOrderID; // exchange order id
    unsigned long ExchangeEventID; // exchange order id
    unsigned long StatusID;        // rejected
    unsigned long TimeStamp;
} CANCEL_ORDER_NOTIFICATION;


typedef struct 
{
    
    unsigned short MessageType;
    unsigned long UserID;
    long TotalNumberOfCancelledOrders;
    unsigned long ExchangeEventID;
    unsigned long ExchangeStatusID;
    unsigned long ExchangeTimeStamp;
} CANCEL_ALL_ORDERS_NOTIFICATION;

typedef struct
{
    unsigned short MessageType; // modifiy request
    unsigned long UserID;       // specifies a unique user id

    unsigned long ExchangeOrderID;             // exchange order id to modify
    unsigned char OrderType;                   // order type limit or market
    unsigned char OrderTimeInForce;            // TIF IOC or DAY
    unsigned char OrderExecutionType;          // PostOnly, CloseOnTrigger, ReduceOnly
    unsigned long OrderQuantity;               // quantity to modify
    unsigned long OrderDisclosedQuantity;      // disclosed quantity to modify
    long OrderPrice;                           // price to modify
    unsigned short MarketPriceProtectionTicks; // 0 - 65535
    long OrderTriggerPrice;                    // trigger price to modify
    unsigned char TriggerOn;                   // LastTradedPrice, MarkPrice
    unsigned long ExchangeEventID;             // exchange order id
    unsigned long StatusID;                    // rejected
    unsigned long TimeStamp;
} MODIFY_ORDER_NOTIFICATION;

//Will remain same will be determined after putting back
typedef struct
{
    unsigned short MessageType;                    // unsolicited notifications
    unsigned long UserID;                          // specifies a unique user id
    unsigned long ExchangeOrderID;
    unsigned long MatchID;                         // match id whatever that is
    unsigned long TradeID;                         // trade id whatever that is
    unsigned long FillID;                          // fill ID whatever that is
    unsigned long InstrumentID;                    // exchange instrument id
    unsigned long FillQuantity;                    // what has been filled
    long          FillPrice;                                // price filled at
    unsigned char OrderSide;                       // side buy or sell
    unsigned long OrderQuantityRemaining;          // quantity remaining
    unsigned long OrderDisclosedQuantityRemaining; // disclosed quantity remaining
    unsigned long ExchangeEventID;                 // exchange order id
    unsigned long StatusID;                        // rejected
    unsigned long TimeStamp;
} FILL_NOTIFICATION;

typedef struct
{
    unsigned short MessageType; // unsolicited notifications
    unsigned long StatusID;     // status good or bad or whatever
    unsigned long TimeStamp;
} SESSION_NOTIFICATION;

typedef struct
{
    unsigned short MessageType;      // unsolicited notifications
    unsigned long UserID;            // specifies a unique user id
    unsigned long UserTag;           // user request identifier
    unsigned long ExchangeOrderID;   // exchange order id
    unsigned long InstrumentID;      // exchange instrument id
    unsigned long OrderTriggerPrice; // exchange trigger price of order
    unsigned long ExchangeEventID;   // exchange order id
    unsigned long StatusID;          // rejected
    unsigned long TimeStamp;
} STOP_TRIGGER_NOTIFICATION;

typedef struct
{
    unsigned short MessageType; // unsolicited notifications
    unsigned long StatusID;     // rejected
    unsigned long TimeStamp;
} GATEWAY_CLIENT_LOGOFF_NOTIFICATION;

typedef struct
{
    unsigned short MessageType; // unsolicited notifications
    unsigned long UserID;       // specifies a unique user id
    unsigned long StatusID;     //
    unsigned long TimeStamp;
} GATEWAY_CLIENT_NOTIFICATION;

typedef struct
{
    unsigned short MessageType;           // new order
    unsigned long UserID;                 // specifies a unique user id
    unsigned long ExchangeOrderID;        // exchange order id to cancel
    unsigned long OrderQuantityRemaining; // wahts left to execute
    unsigned long OrderInstrumentID;      // just some number
    unsigned char OrderType;              // order type limit or market
    unsigned char OrderSide;              // buy or sell
    unsigned long OrderQuantity;          // just a number
    unsigned long OrderDisclosedQuantity; // just a number
    long OrderPrice;                      // price of order, fixed decimal (from instrument definitions)
    long OrderTriggerPrice;               // trigger price of order, fixed decimal (from instrument definitions)
    unsigned char OrderTimeInForce;       // IOC or DAY
    unsigned long StatusID;               // rejected
    unsigned long TimeStamp;
} WORKING_ORDER_NOTIFICATION;





typedef union {
    unsigned short msg_type;
    HEARTBEAT hb;
    GATEWAY_CLIENT_LOGON_REQUEST gclg_req;
    GATEWAY_CLIENT_LOGON_RESPONSE gclg_res;
    GATEWAY_CLIENT_LOGOFF_REQUEST gclo_req;
    GATEWAY_CLIENT_LOGOFF_RESPONSE gclo_res;
    NEW_ORDER_REQUEST nos_req;
    NEW_ORDER_RESPONSE nos_res;
    MODIFY_ORDER_REQUEST mo_req;
    MODIFY_ORDER_RESPONSE mo_res;
    CANCEL_ORDER_REQUEST co_req;
    CANCEL_ORDER_RESPONSE co_res;
    CANCEL_ALL_ORDERS_REQUEST cao_req;
    CANCEL_ALL_ORDERS_RESPONSE cao_res;
    WORKING_ORDERS_REQUEST wo_req;
    WORKING_ORDERS_RESPONSE wo_res;
    FILL_REQUEST fill_req;
    FILL_RESPONSE fill_res;
    CANCEL_ORDER_NOTIFICATION co_not;
    MODIFY_ORDER_NOTIFICATION mo_not;
    FILL_NOTIFICATION fil_not;
    SESSION_NOTIFICATION ses_not;
    STOP_TRIGGER_NOTIFICATION st_not;
    WORKING_ORDER_NOTIFICATION wo_not;
    GATEWAY_CLIENT_LOGOFF_NOTIFICATION gclo_not;
    GATEWAY_CLIENT_NOTIFICATION gc_not;
    MODIFY_LEVERAGE_REQUEST mlev_req;
    MODIFY_LEVERAGE_RESPONSE mlev_res;
    MODIFY_LEVERAGE_NOTIFICATION mlev_not;
} BINARY_INTERFACE_MESSAGES;

typedef struct
{
    FRAME_HDR hdr;
    BINARY_INTERFACE_MESSAGES msg;
} BINARY_FRAME;




enum binary_gateway_interface_message_errors {
    E_no_error = 0,
    E_msg_frame_too_small = 10001,
    E_msg_frame_too_large = 10002,
    E_msg_type_not_valid = 10003,
    E_msg_length_invalid = 10004,
    E_msg_send_in_wrong_state = 10005,
    E_msg_gateway_client_id_already_logged_on = 10006,
    E_msg_gateway_client_id_unknown = 10007,
    E_msg_gateway_client_id_add_failed = 10008,
    E_msg_gateway_client_isnt_logged_on = 10009,
    E_msg_gateway_client_id_logoff_while_logging_on = 10010,
    E_msg_gateway_client_id_logoff_in_progress = 10011,
    E_msg_user_id_unknown = 10012,
    E_msg_user_id_disabled = 10013,
    E_msg_user_id_add_failed = 10014,
    E_msg_exchange_is_offline = 10015,
    E_msg_gateway_client_details_update = 10016,
    E_msg_gateway_client_disabled = 10017,
};

#pragma pack(0)
