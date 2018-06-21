#ifndef RESOURCE_H
#define RESOURCE_H

#include "win_common.h"

#define IDC_STATIC -1

/* Icons */
#define IDI_MAIN                        101
#define IDI_LUA_LOGO                    102

/* Dialog windows */
#define IDD_MAIN_WND                    1000
    #define IDC_ARR_TIME                1001
    #define IDC_LEAVE_TIME              1002
    #define IDC_WORK_TIME               1003

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

#define IDD_LE_WND                      1040
    #define IDC_LUA_EDIT                    1041
    #define IDC_LUA_LOGO_ICON               1042
    #define IDC_LUA_LINK                    1043
    #define IDC_LESTATUSBAR                 1049

/* Menus */
#define IDR_MENU                        4000
    #define IDM_RUNATSTARTUP                4001
    #define IDM_EXIT                        4002
    #define IDM_DBG_WND                     4003
    #define IDM_EDIT                        4004
    #define IDM_ABOUT                       4005

#define IDR_EDITOR_MENU                 4100
    #define IDM_EDITOR_NEW                  4001
    #define IDM_EDITOR_DEFAULT              4002
    #define IDM_EDITOR_OPEN                 4003
    #define IDM_EDITOR_SAVE                 4004
    #define IDM_EDITOR_SAVEAS               4005
    #define IDM_EDITOR_EXIT                 4006
    #define IDM_EDITOR_UNDO                 4007
    #define IDM_EDITOR_CUT                  4008
    #define IDM_EDITOR_COPY                 4009
    #define IDM_EDITOR_PASTE                4010
    #define IDM_EDITOR_DELETE               4011
    #define IDM_EDITOR_SELALL               4012
    #define IDM_EDITOR_RUN                  4013

#define IDR_TRAY_MENU                   4200
    #define IDM_SHOWHIDE                    4201

#define IDR_ACC                         5000

/* Strings */
#define IDS_MAIN_ERROR                  1

/* Data files */
#define IDR_DEFAULT_LUA                 50000

#endif /* RESOURCE_H */
