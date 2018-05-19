#ifndef RESOURCE_H
#define RESOURCE_H

#include "win_common.h"

#define IDC_STATIC -1

/* Icons */
#define IDI_MAIN                        101

#define IDD_MAIN_WND                    1000
    #define IDC_ARR_TIME                1001
    #define IDC_LEAVE_TIME              1002
    #define IDC_WORK_TIME               1003

/* Controls */
#define IDD_ABOUT                       1030
    #define IDC_APP_ICON                    1031
    #define IDC_APP_NAME                    1032
    #define IDC_APP_DESC                    1033
    #define IDC_APP_AUTHOR                  1034
    #define IDC_APP_VER                     1035
    #define IDC_APP_GIT                     1036
    #define IDC_LICTEXT                     1037
    #define IDC_GET_LATEST                  1038
    #define IDC_MAILTO                      1039

/* Menus */
#define IDR_MENU                        40000
    #define IDM_RUNATSTARTUP                40001
    #define IDM_EXIT                        40002
    #define IDM_ABOUT                       40003

#define IDR_TRAY_MENU                   41000
    #define IDM_SHOWHIDE                    41001

/* Strings */
#define IDS_MAIN_ERROR                  1


#endif /* RESOURCE_H */
