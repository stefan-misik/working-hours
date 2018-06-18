#include "lua_edit.h"
#include "resource.h"
#include "dialog_resize.h"
#include "accelerators.h"

/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/

/* Web site to help with Lua */
#define LUA_LINK_URL "http://www.lua.org/"

/* Get debug window data pointer */
#define GetLeWindowData(hWnd) ((LPLEWNDDATA)(GetWindowLongPtr((hWnd), \
                                               GWLP_USERDATA)))


/**
 * @brief Debug window data structure
 * 
 */
typedef struct tagLEWNDDATA
{
    HICON hLuaIcon;                 /**< Lua icon */
    DIALOGRESIZE dr;                /**< Dialog resize information */
} LEWNDDATA, *LPLEWNDDATA;

/**
 * @brief Destroy all objects in a Lua edit window data structure and free
 *        memory taken by this structure
 * 
 * @param[in] lpData Window data to be destroyed
 */
static VOID DestroyLeWndData(
    LPLEWNDDATA lpData
)
{
    DrDestroy(&(lpData->dr));
    if(NULL != lpData->hLuaIcon)
        DestroyIcon(lpData->hLuaIcon);

    if(NULL != lpData)
        HeapFree(g_hHeap, 0, lpData);
}

/**
 * @brief Allocate and initialize Lua edit window data structure
 * 
 * @return New Lua edit window data structure pointer
 */
static LPLEWNDDATA CreateLeWndData(VOID)
{
    LPLEWNDDATA lpData;

    /* Allocate data structure associated with the debug window */
    lpData = HeapAlloc(g_hHeap, 0, sizeof(LEWNDDATA));
    if(NULL == lpData)
        return NULL;

    /* Initialize main window data */
    lpData->hLuaIcon = NULL;
    DrInit(&(lpData->dr), NULL, 0);

    return lpData;
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
    LPLEWNDDATA lpData = (LPLEWNDDATA)lpAdditionalData;
    
    /* Store Pointer to the data structure with the window */
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpData);

    lpData->hLuaIcon = (HICON)LoadImage(
        g_hInstance,
        MAKEINTRESOURCE(IDI_LUA_LOGO),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXICON) * 2,
        GetSystemMetrics(SM_CYICON) * 2,
        0
    );

    if(NULL != lpData->hLuaIcon)
    {
        SendDlgItemMessage(hwnd, IDC_LUA_LOGO_ICON, STM_SETICON, 
            (WPARAM) lpData->hLuaIcon, 0);                        
    }
    
    /* Set the console window font */
    SendDlgItemMessage(hwnd, IDC_LUA_EDIT, WM_SETFONT, 
            (WPARAM)GetStockObject(OEM_FIXED_FONT), (LPARAM)TRUE);

    /* Configure auto-resize */
    if(DrInit(&(lpData->dr), hwnd, 3))
    {
        DrConfigureControl(&(lpData->dr), 0, IDC_LUA_EDIT, DR_ANCHOR_LEFT |
            DR_ANCHOR_TOP | DR_ANCHOR_RIGHT | DR_ANCHOR_BOTTOM);
        DrConfigureControl(&(lpData->dr), 1, IDC_LUA_LOGO_ICON,
            DR_ANCHOR_RIGHT | DR_ANCHOR_BOTTOM);
        DrConfigureControl(&(lpData->dr), 2, IDC_LUA_LINK,
            DR_ANCHOR_RIGHT | DR_ANCHOR_BOTTOM);
    }
    
    /* Register for receiving the accelerators */
    AccRegisterWindow(hwnd);
    
    return TRUE;
}

/**
 * @brief Called when window is to be closed
 * 
 * @param hwnd Debug window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnClose(
    HWND hwnd
)
{
    /* Inform parent that debug window wants to be closed */
    SendMessage(GetParent(hwnd), WM_LEWNDOPENCLOSE, 0, (LPARAM)hwnd);
    
    return TRUE;
}

/**
 * @brief Command sent by a control
 * 
 * @param hwnd Lua editor window handle
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
 * @param hwnd Lua editor window handle
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
    switch(wID)
    {
    case IDM_EDITOR_NEW:
        SetDlgItemText(hwnd, IDC_LUA_EDIT, TEXT(""));
        return TRUE;

    case IDM_EDITOR_OPEN:
        return TRUE;

    case IDM_EDITOR_SAVE:
        return TRUE;

    case IDM_EDITOR_SAVEAS:
        return TRUE;

    case IDM_EDITOR_EXIT:
        OnClose(hwnd);
        return TRUE;

    case IDM_EDITOR_UNDO:
        SendDlgItemMessage(hwnd, IDC_LUA_EDIT, EM_UNDO, 0, 0);
        return TRUE;

    case IDM_EDITOR_CUT:
    case IDM_EDITOR_COPY:
        /* Copy the text HERE */
        
        if(IDM_EDITOR_COPY == wID)
            return TRUE;
        
    case IDM_EDITOR_DELETE:
        SendDlgItemMessage(hwnd, IDC_LUA_EDIT, EM_REPLACESEL, TRUE,
            (LPARAM)TEXT(""));
        return TRUE;
        return TRUE;

    case IDM_EDITOR_PASTE:
        return TRUE;

    case IDM_EDITOR_SELALL:
        SendDlgItemMessage(hwnd, IDC_LUA_EDIT, EM_SETSEL, 0, -1);
        return TRUE;
    
    case IDM_EDITOR_RUN:
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief Notification sent by a control
 * 
 * @param hwnd Lua edit window control
 * @param lpNmhdr Pointer to notification information structure
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnNotify(
    HWND hwnd,
    LPNMHDR lpNmhdr
)
{
    switch(lpNmhdr->code)
    {
    case NM_CLICK:
    case NM_RETURN:
        switch(lpNmhdr->idFrom)
        {
            case IDC_LUA_LINK:
                ShellExecute(NULL, TEXT("open"), TEXT(LUA_LINK_URL), NULL,
                    NULL, SW_SHOW);
                return TRUE;
        }
        break;
    }

    return FALSE;
}

/**
 * @brief Called when window is to be destroyed
 * 
 * @param hwnd Debug window handle
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnDestroy(
    HWND hwnd
)
{
    LPLEWNDDATA lpData = GetLeWindowData(hwnd);

    if(NULL != lpData)
    {
        /* Destroy data associated with the window */
        DestroyLeWndData(lpData);
    }

    /* Remove pointer to non-existing data */
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)NULL);

    /* Unregister from receiving the accelerators */
    AccRegisterWindow(NULL);
    
    return TRUE;
}

/**
 * @brief Sent after window's size has changed
 * 
 * @param hwnd Window handle
 * @param wType Type of the size request
 * @param wWidth New width of the window
 * @param wHeight New height of the window
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnSize(
    HWND hwnd,
    WPARAM wType,
    WORD wWidth,
    WORD wHeight
)
{
    LPLEWNDDATA lpData = GetLeWindowData(hwnd);
    
    /* Resize the status bar */
    SendDlgItemMessage(hwnd, IDC_LESTATUSBAR, WM_SIZE, wType,
        MAKELPARAM(wWidth, wHeight));
    
    /* Auto-resize the controls */
    DrDoResize(&(lpData->dr));
    
    return TRUE;
}

/**
 * @brief Sent to a window when the size or position of the window is about to
 *        change
 * 
 * @param hwnd Window handle
 * @param[in,out] lpMinMax Min-max info
 * 
 * @return TRUE if message is processed
 */
static INT_PTR OnGetMinMaxInfo(
    HWND hwnd,
    LPMINMAXINFO lpMinMax
)
{
    LPLEWNDDATA lpData = GetLeWindowData(hwnd);
    
    lpMinMax->ptMinTrackSize.x = lpData->dr.lOrigWidth;
    lpMinMax->ptMinTrackSize.y = lpData->dr.lOrigHeight;
    
    return TRUE;
}

/**
 * @brief Debug window dialog procedure
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
    
    case WM_NOTIFY:
        return OnNotify(hwnd, (LPNMHDR)lParam);

    case WM_CLOSE:
        return OnClose(hwnd);

    case WM_DESTROY:
        return OnDestroy(hwnd);

    case WM_SIZE:
        return OnSize(hwnd, wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_GETMINMAXINFO:
        return OnGetMinMaxInfo(hwnd, (LPMINMAXINFO)lParam);
    }
    
    return FALSE;
}

/******************************************************************************/
/*                                Public                                      */
/******************************************************************************/

/******************************************************************************/
HWND LeWndCreate(
    HWND hwndParent
)
{
    LPLEWNDDATA lpData;
    HWND hwndDebug;

    /* Allocate main window data structure */
    lpData = CreateLeWndData();
    if(NULL == lpData)
    {
        return NULL;
    }
    
    /* Create main Window */
    hwndDebug = CreateDialogParam(
        g_hInstance,
        MAKEINTRESOURCE(IDD_LE_WND),
        hwndParent,
        DialogProc,
        (LPARAM)lpData
    );
    
    /* Check if window was created*/
    if(NULL == hwndDebug)
    {
        DestroyLeWndData(lpData);
        return NULL;
    }
    else
    {
        /* Show debug window */
        ShowWindow(hwndDebug, SW_SHOW);
        
        return hwndDebug;
    }
}

/******************************************************************************/
VOID LeWndSetCode(
    HWND hwndLuaEdit,
    LPCSTR lpFilename,
    LPCSTR lpLuaCode
)
{
    SetDlgItemTextA(hwndLuaEdit, IDC_LUA_EDIT, lpLuaCode);
}