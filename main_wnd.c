#include "main_wnd.h"
#include "resource.h"
#include "tray_icon.h"
#include "about_dialog.h"
#include "defs.h"
#include "working_hours.h"

/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/

/**
 * @brief Main window data
 */
typedef struct tagMAINWNDDATA
{
    HICON hMainIcon;        /** < Main Icon handle */    
    HMENU hTrayIconMenu;    /** < Menu for the tray icon */
    BOOL bOnStartup;        /** < Is being run on startup */
    HFONT hWorkHoursFont;   /** < Font for the hours worked in the dialog box */
    COLORREF crWorkHoursCol;/** < Color of the working hours */   
} MAINWNDDATA, *LPMAINWNDDATA;

/* Tray icon notification messages  */
#define WM_TRAY_ICON WM_USER + 0

/* ID of the tray icon */
#define TRAY_ICON_ID 0

/* Timer ID for the working hours update */
#define WH_TIMER_ID 10
/* Working hours timer period in milliseconds */
#define WH_TIMER_PERIOD 1000

/* Get Main window data pointer */
#define GetMainWindowData(hWnd) (LPMAINWNDDATA)(GetWindowLongPtr((hWnd), \
                                               GWLP_USERDATA))

/**
 * @brief Procedure called when working hours count is to be updated
 * 
 * @param hwnd Main window handle
 */
static VOID UpdateWorkingHours(
    HWND hwnd
)
{
    LPMAINWNDDATA lpData = GetMainWindowData(hwnd);
    SYSTEMTIME st;
    WHTIME whtArrival, whtNow, whtWorked;
    TCHAR lptstrTimeWorked[64];
    
    /* Get arrival time */
    if(GDT_VALID != SendDlgItemMessage(hwnd, IDC_ARR_TIME, DTM_GETSYSTEMTIME,
        0, (WPARAM)(&st)))
        return;
    /* Convert */
    WhSystimeToWht(&whtArrival, &st);
    
    /* Get current time */
    GetLocalTime(&st);
    /* Convert */
    WhSystimeToWht(&whtNow, &st);
    
    /* Calculate current time spent working */
    WhCalculate(&whtArrival, &whtNow, &whtWorked, &(lpData->crWorkHoursCol));
    /* Convert */
    WhWhtToSystime(&st, &whtWorked);
    
    /* Format time spent working into a string */
    GetTimeFormat(LOCALE_CUSTOM_DEFAULT, 0, &st, TEXT("HH':'mm"),
            lptstrTimeWorked, (sizeof(lptstrTimeWorked)/sizeof(TCHAR)) - 1);
    /* Update time worked control */
    SetDlgItemText(hwnd, IDC_WORK_TIME, lptstrTimeWorked);
    
    /* Invalidate entire working hours counter */
    InvalidateRect(GetDlgItem(hwnd, IDC_WORK_TIME), NULL, TRUE);
    return;
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
    if(bShow)
    {
        SetForegroundWindow(hWnd);
        ShowWindow(hWnd, SW_SHOW);
        
        /* Start working hours update timer */
        SetTimer(hWnd, WH_TIMER_ID, WH_TIMER_PERIOD, NULL);
        /* Update working hours now */
        UpdateWorkingHours(hWnd);
        
        
    }
    else
    {
        /* Stop working hours update timer */
        KillTimer(hWnd, WH_TIMER_ID);
        
        ShowWindow(hWnd, SW_HIDE);
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
		
        /* Ty to write filename of current executable to registry */
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
    
    /* Store Pointer to the data structure with the window */
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpData);

    /* Set icon */
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)lpData->hMainIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)lpData->hMainIcon);

    /* Add tray icon */
    TrayIconAdd(hwnd, TRAY_ICON_ID, WM_TRAY_ICON, lpData->hMainIcon);
    
    /* Check or uncheck the 'Run at startup' menu item */
    IsRegisteredToRunAtStartup(hwnd);
    
    /* Change arrival time format */
    SendDlgItemMessage(hwnd, IDC_ARR_TIME, DTM_SETFORMAT, 0,
        (LPARAM)TEXT("HH':'mm"));
    
    /* Create worked hours font */
    lpData->hWorkHoursFont = CreateFont(52,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH,TEXT("Arial"));
    SendDlgItemMessage(hwnd, IDC_WORK_TIME, WM_SETFONT,
        (WPARAM)lpData->hWorkHoursFont, MAKELPARAM(TRUE, 0));
    
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
    /* Hide the window */
    ShowMainWnd(hwnd, FALSE);
    
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
        UpdateWorkingHours(hwnd);
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
                UpdateWorkingHours(hwnd);
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
 * @param hwndC COntrol window handle
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
        
        case IDM_EXIT:
            DestroyWindow(hwnd);
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
        ShowMainWnd(g_hMainWnd, FALSE);
        return TRUE;
    }
}
