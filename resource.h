#ifndef RESOURCE_H
#define RESOURCE_H

#include "win_common.h"

#define IDC_STATIC -1

/* Icons */
#define IDI_MAIN                        101
#define IDI_LUA_LOGO                    102
#define IDI_PAUSE                       150
#define IDI_RESUME                      151


/* Dialog windows */
#define IDD_MAIN_WND                    1000
    #define IDC_ARR_TIME                1001
    #define IDC_LEAVE_TIME              1002
    #define IDC_PAUSE_TIME              1003
    #define IDC_PLAY_PAUSE              1004
    #define IDC_WORK_TIME               1005

#define IDD_ABOUT                       1020
    #define IDC_APP_ICON                    1021
    #define IDC_APP_NAME                    1022
    #define IDC_APP_DESC                    1023
    #define IDC_APP_AUTHOR                  1024
    #define IDC_APP_VER                     1025
    #define IDC_APP_GIT                     1026
    #define IDC_LICTEXT                     1027
    #define IDC_GET_LATEST                  1028
    #define IDC_MAILTO                      1029

#define IDD_DBG_WND                     1030
    #define IDC_DBGCONSOLE                  1032
    #define IDC_DBGSTATUSBAR                1039

/* Menus */
#define IDR_MENU                        4000
    #define IDM_RUNATSTARTUP                4001
    #define IDM_SETARRIVAL                  4002
    #define IDM_EXIT                        4003
    #define IDM_DBG_WND                     4004
    #define IDM_LOAD_DEFAULT                4005
    #define IDM_RELOAD                      4006
    #define IDM_ABOUT                       4007

#define IDR_TRAY_MENU                   4200
    #define IDM_SHOWHIDE                    4201

/* Strings */
#define IDS_MAIN_ERROR                  1

/* Data files */
#define IDR_DEFAULT_LUA                 50000

#endif /* RESOURCE_H */
