#include <Shlwapi.h>
#include <winuser.h>

#include "main_wnd.h"
#include "resource.h"
#include "tray_icon.h"
#include "about_dialog.h"
#include "defs.h"
#include "working_hours.h"
#include "dbg_wnd.h"

/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/

/**
 * @brief Main window data
 */
typedef struct tagMAINWNDDATA
{
    HWND hwnd;              /**< Main window handle */
    HICON hMainIcon;        /**< Main Icon handle */    
    HMENU hTrayIconMenu;    /**< Menu for the tray icon */
    BOOL bOnStartup;        /**< Is being run on startup */
    HFONT hWorkHoursFont;   /**< Font for the hours worked in the dialog box */
    COLORREF crWorkHoursCol;/**< Color of the working hours */   
    WHTIME whtLastUpdate;   /**< Time of last window update */
    WHTIME whtWorkingTime;  /**< Time spent working */
    BOOL bPauseChangeUpdate;/**< Allow W-H to update when pause time changes */
    WHLUA WhLua;            /**< Working hours state */
    LPSTR lpLuaCode;        /**< String containing the current Lua code */
    BOOL bLuaReady;         /**< Lua state initialized and not being editted */
    HWND hwndDebug;         /**< Debug window handle */
    BOOL bIsPaused;         /**< Is work paused */
    HICON hPauseIcon;       /**< Pause work icon */
    HICON hResumeIcon;      /**< Resume work icon */
} MAINWNDDATA, *LPMAINWNDDATA;

/* Tray icon notification messages  */
#define WM_TRAY_ICON (WM_USER + 0)

/* ID of the tray icon */
#define TRAY_ICON_ID 0

/* Timer ID for the working hours update */
#define WH_TIMER_ID 10
/* Working hours timer period in milliseconds */
#define WH_TIMER_PERIOD 1000
/* Time format used by the working hours */
#define WH_TIME_FORMAT "HH':'mm"

/* Get Main window data pointer */
#define GetMainWindowData(hWnd) ((LPMAINWNDDATA)(GetWindowLongPtr((hWnd), \
                                               GWLP_USERDATA)))

/* Path to file containing the Lua code */
#define WH_LUA_CODE_FILE "working-hours.lua"

/**
 * @brief Number of a windows message sent on creation of the taskbar
 * 
 */
static UINT g_uTakbarCreatedMessage = 0;

/**
 * @brief Update notification icon balloon text
 * 
 * @param hwnd Main window handle
 * 
 * @return FALSE on failure
 */
static BOOL UpdateTryIconText(
    HWND hwnd
)
{
    TCHAR lptstrTimeWorked[64];
    TCHAR lptstrTrayBalloon[64];
    DWORD_PTR lpArgs[] = {(DWORD_PTR)lptstrTimeWorked};
    
    /* Get the current working time */
    if(0 == GetDlgItemText(hwnd, IDC_WORK_TIME, lptstrTimeWorked,
            sizeof(lptstrTimeWorked)/sizeof(TCHAR)))
        return FALSE;
    
    /* Format balloon text */
    if (!FormatMessage(FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY, TEXT(PROJECT_NAME) TEXT(" (%1)"), 0,
            0, lptstrTrayBalloon, sizeof(lptstrTrayBalloon)/sizeof(TCHAR),
            (va_list *)lpArgs))
        return FALSE;
    
    /* Update tray icon balloon */
    TrayUpdateText(hwnd, TRAY_ICON_ID, lptstrTrayBalloon);
    
    return TRUE;
}

/**
 * @brief Get the current time in W-H format
 *
 * @param[out] whtime Current time
 */
static VOID GetCurrentWHTime(LPWHTIME whtime)
{
    SYSTEMTIME st;

    /* Get current time */
    GetLocalTime(&st);

    /* Convert */
    WhSystimeToWht(whtime, &st);
}

/**
 * @brief Read time of arrival from the GUI
 *
 * @param hwnd Main window
 * @param[out] whtime Structure to obtain the time
 * @return TRUE on success
 */
static BOOL ReadArrivalWHTime(HWND hwnd, LPWHTIME whtime)
{
    SYSTEMTIME st;

    if(GDT_VALID != SendDlgItemMessage(hwnd, IDC_ARR_TIME,
        DTM_GETSYSTEMTIME, 0, (WPARAM)(&st)))
    {
        return FALSE;
    }

    /* Convert */
    WhSystimeToWht(whtime, &st);

    return TRUE;
}

/**
 * @brief Update working hours text
 *
 * @param hwnd Main window handle
 * @param[in] whtime Working hours
 */
static VOID UpdateWorkingHoursText(HWND hwnd, LPCWHTIME whtime)
{
    SYSTEMTIME st;
    TCHAR lptstrTimeWorked[64];

    /* Convert */
    WhWhtToSystime(&st, whtime);

    /* Format time spent working into a string */
    if(0 != GetTimeFormat(LOCALE_CUSTOM_DEFAULT, 0, &st,
        TEXT(WH_TIME_FORMAT), lptstrTimeWorked,
        (sizeof(lptstrTimeWorked)/sizeof(TCHAR)) - 1))
    {
        /* Update time worked control */
        SetDlgItemText(hwnd, IDC_WORK_TIME, lptstrTimeWorked);

        /* Invalidate entire working hours counter */
        InvalidateRect(GetDlgItem(hwnd, IDC_WORK_TIME), NULL, FALSE);
    }
}

/**
 * @brief Update leave time text
 *
 * @param hwnd Main window handle
 * @param[in] whtime Leave time
 */
static VOID UpdateLeaveTimeText(HWND hwnd, LPCWHTIME whtime)
{
    SYSTEMTIME st;
    TCHAR lptstrLeaveTime[64];

    /* Convert */
    WhWhtToSystime(&st, whtime);

    /* Format time spent working into a string for tray icon */
    if(0 != GetTimeFormat(LOCALE_CUSTOM_DEFAULT, 0, &st, TEXT(WH_TIME_FORMAT),
        lptstrLeaveTime, (sizeof(lptstrLeaveTime)/sizeof(TCHAR)) - 1))
    {
        /* Update control text */
        SetDlgItemText(hwnd, IDC_LEAVE_TIME, lptstrLeaveTime);
    }
}

/**
 * @brief Get the value of the pause time
 *
 * @param hwnd Main window handle
 * @return Pause time in minutes
 */
static WORD GetPauseTime(
    HWND hwnd
)
{
    BOOL success = FALSE;
    WORD pauseTime;

    pauseTime = GetDlgItemInt(hwnd, IDC_PAUSE_TIME, &success, FALSE);

    return success ? pauseTime : 0;
}

/**
 * @brief Calculate minutes from W-H Time
 *
 * @param[in] whtime Time to convert
 * @return Time in minutes
 */
static WORD WHTimeToMinutes(
    LPCWHTIME whtime
)
{
    return (whtime->wHour * 60) + whtime->wMinute;
}

/**
 * @brief Procedure called when working hours count is to be updated
 * 
 * @param hwnd Main window handle
 * @param bForceUpdate Perform an update regardless the last update time value
 */
static VOID UpdateWorkingHours(
    HWND hwnd,
    BOOL bForceUpdate
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
    WHTIME whtArrival, whtNow, whtNewWorkingTime, whtLeave;
    WORD pauseTime;
    
    /* Check if Lua is ready */
    if(!lpData->bLuaReady)
        return;

    /* Get current time */
    GetCurrentWHTime(&whtNow);

    if(bForceUpdate || lpData->whtLastUpdate.wMinute != whtNow.wMinute ||
        lpData->whtLastUpdate.wHour != whtNow.wHour)
    {
        /* Update last update time */
        lpData->whtLastUpdate = whtNow;

        /* Get arrival time */
        if(!ReadArrivalWHTime(hwnd, &whtArrival))
        {
            return;
        }
        /* Get Pause Time */
        pauseTime = GetPauseTime(hwnd);

        /* Calculate current time spent working */
        if(!WhCalculate(&(lpData->WhLua), &whtArrival, &whtNow, pauseTime,
                &whtNewWorkingTime, &(lpData->crWorkHoursCol)))
        {
            return;
        }

        /* Increment pause time and re-calculate, if paused */
        if (lpData->bIsPaused && !bForceUpdate)
        {
            if (lpData->whtWorkingTime.wMinute != whtNewWorkingTime.wMinute ||
                    lpData->whtWorkingTime.wHour != whtNewWorkingTime.wHour)
            {
                pauseTime += WHTimeToMinutes(&whtNewWorkingTime) -
                        WHTimeToMinutes(&(lpData->whtWorkingTime));
                if(!WhCalculate(&(lpData->WhLua), &whtArrival, &whtNow,
                        pauseTime, &whtNewWorkingTime,
                        &(lpData->crWorkHoursCol)))
                {
                    return;
                }
            }
        }

        /* Remember work time */
        lpData->whtWorkingTime = whtNewWorkingTime;

        /* Update pause time and prevent recursive call of update */
        lpData->bPauseChangeUpdate = FALSE;
        SetDlgItemInt(hwnd, IDC_PAUSE_TIME, pauseTime, FALSE);
        lpData->bPauseChangeUpdate = TRUE;

        UpdateWorkingHoursText(hwnd, &whtNewWorkingTime);

        /* Update tray icon balloon */
        UpdateTryIconText(hwnd);

        /* Calculate leave time */
        if(!WhLeaveTime(&(lpData->WhLua), &whtArrival, pauseTime, &whtLeave))
        {
            return;
        }

        UpdateLeaveTimeText(hwnd, &whtLeave);
    }
}

/**
 * @brief Get the handle of the monitor with the mouse cursor on 
 *
 * @ret Monitor handle
 */
static HMONITOR MonitorFromCursor(
    VOID
)
{
    POINT pt;
    if(!GetCursorPos(&pt))
    {
        pt.x = 0;
        pt.y = 0;
    }
    
    return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
}
/**
 * @brief Place the window in the center of the screen with mouse cursor
 *
 * @param hWnd Handle of the window to be moved
 */
static VOID CenterWindow(
	HWND hWnd
)
{
    MONITORINFO mi;

    /* Move windows to the center of monitor */
    mi.cbSize = sizeof(MONITORINFO);
    if(FALSE != GetMonitorInfo(MonitorFromCursor(), &mi))
    {
        RECT rc;
        if(TRUE == GetWindowRect(hWnd, &rc))
        {
            LONG lWidth = rc.right - rc.left;
            LONG lHeight = rc.bottom - rc.top;

            MoveWindow(hWnd,
                ((mi.rcWork.right - mi.rcWork.left) / 2) +
                    mi.rcWork.left - (lWidth / 2),
                ((mi.rcWork.bottom - mi.rcWork.top) / 2) +
                    mi.rcWork.top - (lHeight / 2),
                lWidth,
                lHeight,
                TRUE);
        }
    }
}

/**
 * @brief Show or hide main window
 * 
 * @param hWnd Main window handle
 * @param bShow FALSE to hide the window
 */
static VOID ShowMainWnd(
    HWND hWnd,
    BOOL bShow
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hWnd);
    
    if(bShow)
    {
        SetForegroundWindow(hWnd);
        ShowWindow(hWnd, SW_SHOW);
        if(NULL != lpData->hwndDebug)
            ShowWindow(lpData->hwndDebug, SW_SHOW);
        
        /* Move windows to the screen with cursor */
        CenterWindow(hWnd);
    }
    else
    {
        ShowWindow(hWnd, SW_HIDE);
        if(NULL != lpData->hwndDebug)
            ShowWindow(lpData->hwndDebug, SW_HIDE);
    }
}

/**
 * @brief Destroy all objects in a Main Window data structure and free memory
 *        taken by this structure
 */
static VOID DestroyMainWndData(
    LPMAINWNDDATA lpData
)
{
    if(NULL != lpData)
    {
        if(NULL != lpData->hMainIcon)
            DestroyIcon(lpData->hMainIcon);
        if(NULL != lpData->hTrayIconMenu)
            DestroyMenu(lpData->hTrayIconMenu);
        if(NULL != lpData->hWorkHoursFont)
            DeleteObject(lpData->hWorkHoursFont);
        WhLuaDestroy(&(lpData->WhLua));
        if(NULL != lpData->lpLuaCode)
            HeapFree(g_hHeap,0, lpData->lpLuaCode);
        if(NULL != lpData->hwndDebug)
            DestroyWindow(lpData->hwndDebug);
        if(NULL != lpData->hPauseIcon)
            DestroyIcon(lpData->hPauseIcon);
        if(NULL != lpData->hResumeIcon)
            DestroyIcon(lpData->hResumeIcon);


        HeapFree(g_hHeap, 0, lpData);
    }
}

/**
 * @brief Allocate and initialize main window data structure
 * 
 * @return New Main window data structure pointer
 */
static LPMAINWNDDATA CreateMainWndData(VOID)
{
    LPMAINWNDDATA lpData;

    /* Allocate data structure associated with the main window */
    lpData = HeapAlloc(g_hHeap, 0, sizeof(MAINWNDDATA));
    if(NULL == lpData)
        return NULL;

    /* Initialize main window data */
    lpData->hMainIcon = NULL;
    lpData->hTrayIconMenu = NULL;
    lpData->bOnStartup = FALSE;
    lpData->hWorkHoursFont = NULL;
    lpData->crWorkHoursCol = (COLORREF)GetSysColor(COLOR_BTNTEXT);
    lpData->whtLastUpdate.wHour = 0;
    lpData->whtLastUpdate.wMinute = 0;
    lpData->whtWorkingTime.wHour = 0;
    lpData->whtWorkingTime.wMinute = 0;
    lpData->bPauseChangeUpdate = FALSE;
    WhLuaInit(&(lpData->WhLua));
    lpData->lpLuaCode = NULL;
    lpData->bLuaReady = FALSE;
    lpData->hwndDebug = NULL;
    lpData->bIsPaused = FALSE;
    lpData->hPauseIcon = NULL;
    lpData->hResumeIcon = NULL;


    return lpData;
}

/**
 * @brief Process 'Run at startup' menu selection
 * 
 * @param hwnd Main window handle
 * @param bOnStartup Run on startup
 * 
 * @return TRUE on success
 */
static BOOL OnRunAtStartup(
    HWND hwnd,
    BOOL bOnStartup
)
{
    LPMAINWNDDATA lpData;
    HKEY hKey;

    /* Get main window data */
    lpData = GetMainWindowData(hwnd);

	/* Open Windows/Run key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
		KEY_SET_VALUE | KEY_WOW64_32KEY, &hKey) != 
		ERROR_SUCCESS)
	{
		return FALSE;
	}
    
    if (bOnStartup)
    {
        DWORD dwRes;
        static TCHAR lpExeName[1024];

	/* Get path to current EXE file */
        dwRes = GetModuleFileName(NULL, lpExeName, 1024);

        /* Return, if filename was nor received correctly */
        if (dwRes == 0 && dwRes >= 1024)
        {
            RegCloseKey(hKey);
            return FALSE;
        }	
		
        /* Try to write filename of current executable to registry */
        if (ERROR_SUCCESS != RegSetValueEx(hKey, lpProjectName, 0,
            REG_SZ, (LPBYTE)lpExeName, (dwRes + 1) * sizeof(TCHAR)))
        {
            RegCloseKey(hKey);			
            return FALSE;
        }
    }
    else
    {
        /* Delete registry value holding filename of VUT Disk Mapper */
	RegDeleteValue(hKey, lpProjectName);
    }
    
    /* Check or uncheck the menu item */
    CheckMenuItem(GetMenu(hwnd), IDM_RUNATSTARTUP,
        MF_BYCOMMAND | (bOnStartup ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(lpData->hTrayIconMenu, IDM_RUNATSTARTUP,
        MF_BYCOMMAND | (bOnStartup ? MF_CHECKED : MF_UNCHECKED));

    /* Remember the settings */
    lpData->bOnStartup = bOnStartup;

    /* Close all registry keys */
    RegCloseKey(hKey);
    return TRUE;
}

/**
 * @brief Replace Lua code buffer and load new Lua state
 * 
 * The function will free the original buffer, if any is present and load it
 * in a new Lua state.
 * 
 * @param[in,out] lpData Main window data structure
 * @param[in] lpNewLuaCode Buffer containing the new Lua code
 */
static BOOL LuaLoadCode(
    LPMAINWNDDATA lpData,
    LPSTR lpNewLuaCode
)
{
    if(NULL != lpData->lpLuaCode)
    {
        HeapFree(g_hHeap, 0, lpData->lpLuaCode);
    }
    
    lpData->lpLuaCode = lpNewLuaCode;
    
    /* Initialize Lua state */
    lpData->bLuaReady = WhLuaReset(&(lpData->WhLua));
    if(lpData->bLuaReady)
    {
        if(!WhLuaDoString(&(lpData->WhLua), "", lpData->lpLuaCode))
        {
            WhLuaErrorMessage(&(lpData->WhLua));
            return FALSE;
        }
    }
    return lpData->bLuaReady;
}

/**
 * @brief Reload Lua code either form file or the default
 * 
 * @param[in,out] lpData Window data structure
 * 
 * @return @c TRUE on success
 */
static BOOL ReloadLuaCode(
    LPMAINWNDDATA lpData
)
{
    LPSTR lpLuaCode;

    /* Load the default Lua code */
    lpLuaCode = WhLuaLoadCode(TEXT(WH_LUA_CODE_FILE));
    if(NULL == lpLuaCode)
        lpLuaCode = WhLuaLoadDefaultCode();
    /* Set the loaded Lua code */
    return LuaLoadCode(lpData, lpLuaCode);
}


/**
 * @brief Handle menu item to load default code
 * 
 * @param hwnd Main window handle
 */
static VOID OnScriptLoadDefault(
    HWND hwnd
)
{
    LPSTR lpLuaCode;
    LPMAINWNDDATA lpData;
    
    /* Get main window data */
    lpData = GetMainWindowData(hwnd);
    
    lpLuaCode = WhLuaLoadDefaultCode();
    
    if(LuaLoadCode(lpData, lpLuaCode) &&
            WhLuaSaveCode(TEXT(WH_LUA_CODE_FILE), lpLuaCode))
    {
        MessageBox(hwnd, TEXT("Successfully loaded the default Lua script. "
                    "Default script stored in '" WH_LUA_CODE_FILE "' file."),
                TEXT("Lua Export"), MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBox(hwnd, TEXT("Could not load the default Lua script."),
                TEXT("Lua Export"), MB_OK | MB_ICONWARNING);   
    }
}

/**
 * @brief Handle menu item to reload Lua code
 * 
 * @param hwnd Main window handle
 */
static VOID OnScriptReload(
    HWND hwnd
)
{
    LPMAINWNDDATA lpData;
    /* Get main window data */
    lpData = GetMainWindowData(hwnd);
    
    if(ReloadLuaCode(lpData))
    {
        MessageBox(hwnd, TEXT("Successfully reloaded the Lua script."),
                TEXT("Lua Reload"), MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBox(hwnd, TEXT("Could not reload the Lua script."),
                TEXT("Lua Reload"), MB_OK | MB_ICONWARNING);   
    }
}

/**
 * @brief Show the window displaying Lua debug messages
 * 
 * @param hwnd Main window handle
 * @param bShow Show or hide the debug window
 */
static VOID OnDbgWnd(
    HWND hwnd,
    BOOL bShow
)
{
    LPMAINWNDDATA lpData;
    /* Get main window data */
    lpData = GetMainWindowData(hwnd);
    
    if(bShow)
    {
        /* Destroy already existing window */
        if(NULL != lpData->hwndDebug)
        {
            DestroyWindow(lpData->hwndDebug);
        }

        lpData->hwndDebug = DbgWndCreate(hwnd);
    }
    else
    {
        if(NULL != lpData->hwndDebug)
        {
            DestroyWindow(lpData->hwndDebug);
            lpData->hwndDebug = NULL;
        }
    }
    
    if(NULL != lpData->hwndDebug)
    {
        CheckMenuItem(GetMenu(hwnd), IDM_DBG_WND, MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        CheckMenuItem(GetMenu(hwnd), IDM_DBG_WND, MF_BYCOMMAND | MF_UNCHECKED);
    }
    
    /* Inform Lua module about debug window status change */
    WhLuaSetDebugWnd(&(lpData->WhLua), lpData->hwndDebug);
}

/**
 * @brief Check if application is registered to run at startup
 * 
 * @param hwnd Main window handle
 */
static VOID IsRegisteredToRunAtStartup(
    HWND hwnd
)
{
    LPMAINWNDDATA lpData;
    HKEY hKey;
    LONG lRes;

    /* Get main window data */
    lpData = GetMainWindowData(hwnd);
	
    /* Open Windows/Run key */
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
        KEY_QUERY_VALUE | KEY_WOW64_32KEY, &hKey) != 
        ERROR_SUCCESS)
    {		
        return;
    }

    /* Check if specified registry value exists */
    lRes = RegQueryValueEx(hKey, lpProjectName, NULL,
        NULL, NULL, NULL);

    /* Close all registry keys */
    RegCloseKey(hKey);	

    /* If value exists */
    lpData->bOnStartup = (ERROR_SUCCESS == lRes);
    
    /* Check or uncheck the menu item */
    CheckMenuItem(GetMenu(hwnd), IDM_RUNATSTARTUP,
        MF_BYCOMMAND | ((lpData->bOnStartup) ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(lpData->hTrayIconMenu, IDM_RUNATSTARTUP,
        MF_BYCOMMAND | ((lpData->bOnStartup) ? MF_CHECKED : MF_UNCHECKED));
}

/**
 * @brief Set arrival time  to current time
 *
 * @param hwnd Main window hadle
 */
static VOID SetArrivalToNow(
    HWND hwnd
)
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    SendDlgItemMessage(hwnd, IDC_ARR_TIME, DTM_SETSYSTEMTIME, GDT_VALID,
            (LPARAM)(&now));
}

/******************************************************************************/
/*                         Windows Messages                                   */
/******************************************************************************/

/**
 * @brief Process WM_INITDIALOG message
 * 
 * @param hwndInitialControl A handle to the control to receive the default
 *        keyboard focus
 * @param[in] lpData Additional initialization data
 * @return Set the keyboard focus
 */
static BOOL OnInitDialog(
    HWND hwnd,
    HWND hwndInitialControl,
    LPVOID lpAdditionalData
)
{
    LPMAINWNDDATA lpData = (LPMAINWNDDATA)lpAdditionalData;
    
    /* Store window handle */
    lpData->hwnd = hwnd;
    
    /* Store Pointer to the data structure with the window */
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpData);

    /* Set icon */
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)lpData->hMainIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)lpData->hMainIcon);
    
    /* Set play pause default icon */
    SendDlgItemMessage(hwnd, IDC_PLAY_PAUSE, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM)lpData->hPauseIcon);

    /* Add tray icon */
    TrayIconAdd(hwnd, TRAY_ICON_ID, WM_TRAY_ICON, lpData->hMainIcon);
    
    /* Check or uncheck the 'Run at startup' menu item */
    IsRegisteredToRunAtStartup(hwnd);
    
    /* Configure Pause time control */
    SetDlgItemInt(hwnd, IDC_PAUSE_TIME, 0, FALSE);
    SendDlgItemMessage(hwnd, IDC_PAUSE_TIME_SPIN, UDM_SETBUDDY,
            (WPARAM)GetDlgItem(hwnd, IDC_PAUSE_TIME), 0);
    SendDlgItemMessage(hwnd, IDC_PAUSE_TIME_SPIN, UDM_SETRANGE32, 0, 60 * 24);

    /* Change arrival time format */
    SendDlgItemMessage(hwnd, IDC_ARR_TIME, DTM_SETFORMAT, 0,
        (LPARAM)TEXT(WH_TIME_FORMAT));
    SetArrivalToNow(hwnd);
    
    /* Create worked hours font */
    lpData->hWorkHoursFont = CreateFont(46, 0, 0, 0, FW_BOLD, FALSE, FALSE,
        FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    SendDlgItemMessage(hwnd, IDC_WORK_TIME, WM_SETFONT,
        (WPARAM)lpData->hWorkHoursFont, MAKELPARAM(TRUE, 0));
    
    /* Set window handle for parenting Lua error message-boxes */
    WhLuaSetErrorParentWnd(&(lpData->WhLua), hwnd);
    /* Reload Lua code form file or default */
    ReloadLuaCode(lpData);
            
    /* Start working hours update timer */
    SetTimer(hwnd, WH_TIMER_ID, WH_TIMER_PERIOD, NULL);
    /* Force an update working hours and leave time now */
    UpdateWorkingHours(hwnd, TRUE);

    return TRUE;
}

/**
 * @brief called when window is to be closed
 * 
 * @param hwnd Main window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnClose(
    HWND hwnd
)
{
#ifndef _DEBUG
    /* Hide the window */
    ShowMainWnd(hwnd, FALSE);
#else
    DestroyWindow(hwnd);
#endif

    return TRUE;
}

/**
 * @brief Called when window is to be destroyed
 * 
 * @param hwnd Main window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnDestroy(
    HWND hwnd
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
    
    /* Remove tray icon */
    TrayIconRemove(hwnd, TRAY_ICON_ID);
    
    if(NULL != lpData)
    {
        /* Destroy data associated with the window */
        DestroyMainWndData(lpData);
    }

    /* Remove pointer to non-existing data */
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)NULL);

    /* Quit the application */
    PostQuitMessage(0);

    return TRUE;
}

/**
 * @brief Received when a timer elapses
 * 
 * @param hwnd Main window handle
 * @param idEvent Timer ID
 * @param lpTimerProc Pointer to procedure registered with the timer
 * 
 * @return TRUE if message is processed 
 */
static INT_PTR OnTimer(
    HWND hwnd,
    UINT_PTR idEvent,
    TIMERPROC lpTimerProc
)
{
    switch(idEvent)
    {
    case WH_TIMER_ID:
        UpdateWorkingHours(hwnd, FALSE);
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief Notification sent by a control
 * 
 * @param hwnd Main window control
 * @param lpNmhdr Pointer to notification information structure
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnNotify(
    HWND hwnd,
    LPNMHDR lpNmhdr
)
{
    switch(lpNmhdr->idFrom)
    {
        case IDC_ARR_TIME:
            if(DTN_DATETIMECHANGE == lpNmhdr->code)
            {
                /* Update time spent working when change of arrival time has 
                 * occurred */
                UpdateWorkingHours(hwnd, TRUE);
                return TRUE;
            }
            else
                return FALSE;
    }

    return FALSE;
}

/**
 * @brief Command sent by a control
 * 
 * @param hwnd Main window handle
 * @param wNotifCode Control-specific notification code
 * @param wControlID Dialog Control ID
 * @param hwndC Control window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnControlCommand(
    HWND hwnd,
    WORD wNotifCode,
    WORD wControlID,
    HWND hwndC
)
{
    switch(wControlID)
    {
    case IDC_WORK_TIME:
        /* Deselect this control and take focus away from this control */
		if(GetFocus() == GetDlgItem(hwnd, IDC_WORK_TIME))
		{
            SendDlgItemMessage(hwnd, IDC_WORK_TIME, EM_SETSEL, (WPARAM)-1,
			    (LPARAM)0);
            SetFocus(GetDlgItem(hwnd, IDC_ARR_TIME));
		}
        return FALSE;
    
    case IDC_PLAY_PAUSE:
    {
        LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
        if (lpData->bIsPaused)
        {
            lpData->bIsPaused = FALSE;
            SendDlgItemMessage(hwnd, IDC_PLAY_PAUSE, BM_SETIMAGE, IMAGE_ICON,
                    (LPARAM)lpData->hPauseIcon);
            EnableWindow(GetDlgItem(hwnd, IDC_PAUSE_TIME), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_PAUSE_TIME_SPIN), TRUE);
        }
        else
        {
            lpData->bIsPaused = TRUE;
            SendDlgItemMessage(hwnd, IDC_PLAY_PAUSE, BM_SETIMAGE, IMAGE_ICON,
                    (LPARAM)lpData->hResumeIcon);
            EnableWindow(GetDlgItem(hwnd, IDC_PAUSE_TIME), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_PAUSE_TIME_SPIN), FALSE);
        }
        return TRUE;
    }

    case IDC_PAUSE_TIME:
        if (EN_CHANGE == wNotifCode)
        {
            LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
            /* Update, only when it is allowed to prevent stack overflow by
             * recursively calling update function */
            if (lpData->bPauseChangeUpdate)
            {
                UpdateWorkingHours(hwnd, TRUE);
            }
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

/**
 * @brief Command sent by a menu or an accelerator
 * 
 * @param hwnd Main window handle
 * @param wID Menu item or accelerator ID
 * @param bIsMenu TRUE if sent by menu item
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnMenuAccCommand(
    HWND hwnd,
    WORD wID,
    BOOL bIsMenu
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
    
    switch(wID)
    {
        case IDM_RUNATSTARTUP:
            if(bIsMenu)
                OnRunAtStartup(hwnd, !(lpData->bOnStartup));
            return TRUE;

        case IDM_SETARRIVAL:
            if(bIsMenu)
            {
                SetArrivalToNow(hwnd);
                UpdateWorkingHours(hwnd, TRUE);
            }
            return TRUE;

        case IDM_EXIT:
            DestroyWindow(hwnd);
            return TRUE;
            
        case IDM_LOAD_DEFAULT:
            OnScriptLoadDefault(hwnd);
            return TRUE;
            
        case IDM_RELOAD:
            OnScriptReload(hwnd);
            return TRUE;
            
        case IDM_DBG_WND:
            OnDbgWnd(hwnd, (NULL == lpData->hwndDebug));
            return TRUE;

        case IDM_ABOUT:
            ShowAboutDialog(hwnd);
            return TRUE;

        case IDM_SHOWHIDE:
            ShowMainWnd(hwnd, !IsWindowVisible(hwnd));
            return TRUE;
    }
    return FALSE;
}

/**
 * @brief On system command
 * 
 * @param hwnd Main window handle
 * @param wType Type of the system command
 * @param wMouseX Mouse X position, if command chosen by a mouse
 * @param wMouseY Mouse Y position, if command chosen by a mouse
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnSysCommand(
    HWND hwnd,
    WPARAM wType,
    WORD wMouseX,
    WORD wMouseY
)
{
    switch(wType)
    {
        case SC_MINIMIZE:
            /* Hide the window instead of minimizing it */
            ShowMainWnd(hwnd, FALSE);
            return TRUE;
    }

    return FALSE;
}

/**
 * @brief On Tray icon notification
 * 
 * @param hwnd Main window handle
 * @param wID Tray Icon ID
 * @param uMsg Mouse or keyboard message associated with the event
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnTrayIconNotify(
    HWND hwnd,
    WPARAM wID,
    UINT uMsg
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);

    switch(uMsg)
    {
    case WM_LBUTTONDBLCLK:
        ShowMainWnd(hwnd, !IsWindowVisible(hwnd));
        return TRUE;

    case WM_RBUTTONUP:
        TrayIconShowPopupMenu(hwnd, lpData->hTrayIconMenu);
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief Requests color of disabled edit control child
 * 
 * @param hwnd Main window handle
 * @param hdcControl Device context of the control
 * @param hwndControl Control window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnCtlColorStatic(
    HWND hwnd,
    HDC hdcControl,
    HWND hwndControl
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
    
    switch(GetDlgCtrlID(hwndControl))
    {
    case IDC_WORK_TIME:
        SetBkMode(hdcControl, (COLORREF)GetSysColor(COLOR_BTNFACE));
        SetTextColor(hdcControl, lpData->crWorkHoursCol);
        return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
    }
    
    return FALSE;
}

/**
 * @brief Debug window request to open/close
 * 
 * @param hwnd Main window handle
 * @param hwndDebug Debug window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnDbgWndOpenClose(
    HWND hwnd,
    HWND hwndDebug
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
    
    if (NULL == hwndDebug)
    {
        OnDbgWnd(hwnd, TRUE);
    }
    else if(hwndDebug == lpData->hwndDebug)
    {
        OnDbgWnd(hwnd, FALSE);
    }
    
    return TRUE;
}

/**
 * @brief Message sent to top level windows when taskbar is created
 * 
 * @param hwnd Main window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnTaskbarCreated(
    HWND hwnd
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
    
    /* Re-add tray icon */
    TrayIconAdd(hwnd, TRAY_ICON_ID, WM_TRAY_ICON, lpData->hMainIcon);
    /* Update the tray icon text */
    UpdateTryIconText(hwnd);
    
    return TRUE;
}

/**
 * @brief Main window dialog procedure
 * 
 * @param hwnd Main Window handle
 * @param uMsg Message to process
 * @param wParam WParam
 * @param lParam LParam
 * 
 * @return Depending on message
 */
static INT_PTR CALLBACK DialogProc(
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        return OnInitDialog(hwnd, (HWND)wParam, (LPVOID)lParam);
            
    case WM_NOTIFY:
        return OnNotify(hwnd, (LPNMHDR)lParam);
            
    case WM_COMMAND:
        /* Control command */
        if(0 != lParam)
        {
            return OnControlCommand(hwnd, HIWORD(wParam),
                    LOWORD(wParam), (HWND)lParam);
        }
        /* Menu or accelerator command */
        else
        {
            return OnMenuAccCommand(hwnd, LOWORD(wParam),
                    (HIWORD(wParam) == 0));
        }

    case WM_SYSCOMMAND:
        return OnSysCommand(hwnd, wParam, GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam));

    case WM_CLOSE:
        return OnClose(hwnd);

    case WM_DESTROY:
        return OnDestroy(hwnd);
    
    case WM_TIMER:
        return OnTimer(hwnd, (UINT_PTR)wParam, (TIMERPROC)lParam);

    case WM_TRAY_ICON:
        return OnTrayIconNotify(hwnd, wParam, (UINT)lParam);
    
    case WM_CTLCOLORSTATIC:
        return OnCtlColorStatic(hwnd, (HDC)wParam, (HWND)lParam);
        
    case WM_DBGWNDOPENCLOSE:
        return OnDbgWndOpenClose(hwnd, (HWND)lParam);
        
    default:
        if(g_uTakbarCreatedMessage == uMsg)
            return OnTaskbarCreated(hwnd);

        break;
    }
    
    return FALSE;
}

/******************************************************************************/
/*                                Public                                      */
/******************************************************************************/

/******************************************************************************/
HWND g_hMainWnd = NULL;

/******************************************************************************/
BOOL CreateMainWindow(
    int nCmdShow
)
{
    LPMAINWNDDATA lpData;
    
    /* Get the taskbar creation message number */
    g_uTakbarCreatedMessage = TrayIconTaskbarCreatedMessage();
    
    if(NULL != g_hMainWnd)
    {
        return FALSE;
    }
    
    /* Allocate main window data structure */
    lpData = CreateMainWndData();
    if(NULL == lpData)
    {
        return FALSE;
    }

    /* Load main icon */
    lpData->hMainIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
    if(NULL == lpData->hMainIcon)
    {
        DestroyMainWndData(lpData);
        return FALSE;
    }
    
    /* Load other icons */
    lpData->hPauseIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_PAUSE),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
            LR_DEFAULTCOLOR);
    lpData->hResumeIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_RESUME),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
            LR_DEFAULTCOLOR);

    /* Load tray icon menu */
    lpData->hTrayIconMenu = LoadMenu(g_hInstance,
            MAKEINTRESOURCE(IDR_TRAY_MENU));
    if(NULL == lpData->hTrayIconMenu)
    {
        DestroyMainWndData(lpData);
        return FALSE;
    }

    /* Set default tray menu item */
    SetMenuDefaultItem(GetSubMenu(lpData->hTrayIconMenu, 0), IDM_SHOWHIDE,
        FALSE);
    
    /* Create main Window */
    g_hMainWnd = CreateDialogParam(
        g_hInstance,
        MAKEINTRESOURCE(IDD_MAIN_WND),
        NULL,
        DialogProc,
        (LPARAM)lpData
    );
    
    /* Check if window was created*/
    if(NULL == g_hMainWnd)
    {
        DestroyMainWndData(lpData);
        return FALSE;
    }
    else
    {
        /* Hide the window by default */
		#ifndef _DEBUG
        ShowMainWnd(g_hMainWnd, FALSE);
		#else
        ShowMainWnd(g_hMainWnd, TRUE);
		#endif
        return TRUE;
    }
}
