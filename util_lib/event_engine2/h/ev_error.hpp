#pragma once




enum ev_errors
    {
    E_ev_no_error = 0,
    E_ev_clear_failed = 1000,
    E_ev_get_flags = 1001,
    E_ev_set_flags = 1002,
    E_ev_fd_value_range = 1003,
    E_ev_event_fd_open = 1004,
    E_ev_poke_failed = 1005,
    E_ev_fd_type_illegal = 1006,
    E_ev_fd_in_use = 1007,
    E_ev_fd_not_in_use = 1008,
    E_ev_fd_too_many = 1009,
    E_ev_fd_event_illegal = 1010,
    E_ev_null_ptr = 1011,
    E_ev_invalid_position = 1012,
    E_ev_same_thread = 1013,
    E_ev_not_active = 1014,
    E_ev_tmr_type = 1015,
    E_ev_tmr_too_many = 1016,
    E_ev_tmr_add_duplicate = 1017,
    E_ev_tmr_not_found = 1018,
    E_ev_tmr_value_range = 1019,
    E_ev_tmr_type_illegal = 1020,
    E_ev_tmr_timeout_range = 1021,
    E_ev_tmr_create = 1022,
    E_ev_tmr_start = 1023,
    E_ev_too_many_tasks = 1024,
    E_ev_task_corrupt = 1025,
    E_ev_epoll_failed = 1026,
    E_ev_epoll_ctl = 1027,
    E_ev_event_illegal = 1028,
    E_ev_illegal_task_id = 1029,
    E_ev_invalid_task_name = 1030,
    E_ev_task_name_not_found = 1031,
    E_ev_event_range = 1032,
    };



