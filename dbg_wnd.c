#include "dbg_wnd.h"
#include "resource.h"
#include "dialog_resize.h"
#include <Strsafe.h>
#include <winerror.h>
#include <minwinbase.h>

/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/

#define DBG_BUFFER_LEN (16 * 1024)

/* Get debug window data pointer */
#define GetDbgWindowData(hWnd) ((LPDBGWNDDATA)(GetWindowLongPtr((hWnd), \
                                               GWLP_USERDATA)))

/* Calculate free space in the debug log buffer */
#define GetFreeBufferSpace(lpData) ((DBG_BUFFER_LEN) - (lpData)->iLogLength - 1)
/**
 * @brief Debug window data structure
 * 
 */
typedef struct tagDBGWNDDATA
{
    HWND hwnd;                      /**< Debug window handle */
    CHAR strBuffer[DBG_BUFFER_LEN]; /**< Buffer for debug messages */
    INT iLogLength;                 /**< Number of characters in log up to the
                                     *   terminating zero */
    DIALOGRESIZE dr;                /**< Dialog resize information */
} DBGWNDDATA, *LPDBGWNDDATA;

/**
 * @brief Destroy all objects in a debug window data structure and free memory
 *        taken by this structure
 * 
 * @param[in] lpData Window data to be destroyed
 */
static VOID DestroyDbgWndData(
    LPDBGWNDDATA lpData
)
{
    if(NULL != lpData)
    {
        DrDestroy(&(lpData->dr));

        HeapFree(g_hHeap, 0, lpData);
    }
}

/**
 * @brief Allocate and initialize debug window data structure
 * 
 * @return New debug window data structure pointer
 */
static LPDBGWNDDATA CreateDbgWndData(VOID)
{
    LPDBGWNDDATA lpData;

    /* Allocate data structure associated with the debug window */
    lpData = HeapAlloc(g_hHeap, 0, sizeof(DBGWNDDATA));
    if(NULL == lpData)
        return NULL;

    /* Initialize main window data */
    lpData->iLogLength = 0;
    lpData->strBuffer[lpData->iLogLength] = '\0';
    DrInit(&(lpData->dr), NULL, 0);

    return lpData;
}

/**
 * @brief Make sure buffer can take a message with provided length
 * 
 * Rolls log buffer up to make space for the message with specified length.
 * 
 * @param[in,out] lpData Debug window data structure
 * @param iSpace The length of the message
 * 
 * @return FALSE if not saturated by buffer length, otherwise - saturated by
 *         buffer length
 */
static BOOL EnsureBuffer(
    LPDBGWNDDATA lpData,
    INT iSpace
)
{
    INT iRollDistance;
    BOOL bRet = FALSE;
    
    /* Saturatre the requested space on buffer length (-1 for terminatig zeo) */
    if(iSpace > (DBG_BUFFER_LEN - 1))
    {
        iSpace = (DBG_BUFFER_LEN - 1);
        bRet = TRUE;
    }
    
    /* Calcualte the missing space */
    iRollDistance = iSpace - GetFreeBufferSpace(lpData);
    
    if(iRollDistance > 0)
    {
        LPSTR lpBufferPos = lpData->strBuffer;
        INT iRollPos;
        
        /* Saturate the roll distance */
        if(iRollDistance > lpData->iLogLength)
            iRollDistance = lpData->iLogLength;
        
        /* Temporarily put new line character at the end of the log ton allow
         * following loop to stop at the end of the log */
        lpData->strBuffer[lpData->iLogLength] = '\n';
        while(lpData->strBuffer[iRollDistance] != '\n')
            iRollDistance ++;
        
        /* Also remove the new line character \n */
        if(iRollDistance < lpData->iLogLength)
            iRollDistance ++;
        
        /* Move the remainder of the logs */
        iRollPos = iRollDistance;
        while(iRollPos < lpData->iLogLength)
            *(lpBufferPos ++) = lpData->strBuffer[iRollPos ++];
        
        /* Update log buffer string information */
        lpData->iLogLength -= iRollDistance;
        lpData->strBuffer[lpData->iLogLength] = '\0';
    }
    
    return bRet;
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
    LPDBGWNDDATA lpData = (LPDBGWNDDATA)lpAdditionalData;
    
    /* Store window handle */
    lpData->hwnd = hwnd;
    
    /* Store Pointer to the data structure with the window */
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpData);
    
    /* Set the console window font */
    SendDlgItemMessage(hwnd, IDC_DBGCONSOLE, WM_SETFONT, 
            (WPARAM)GetStockObject(ANSI_FIXED_FONT), (LPARAM)TRUE);
    
    /* Configure auto-resize */
    if(DrInit(&(lpData->dr), hwnd, 1))
    {
        DrConfigureControl(&(lpData->dr), 0, IDC_DBGCONSOLE, DR_ANCHOR_LEFT |
            DR_ANCHOR_TOP | DR_ANCHOR_RIGHT | DR_ANCHOR_BOTTOM);
    }
    
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
    SendMessage(GetParent(hwnd), WM_DBGWNDOPENCLOSE, 0, (LPARAM)hwnd);
    
    return TRUE;
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
    LPDBGWNDDATA lpData = GetDbgWindowData(hwnd);

    if(NULL != lpData)
    {
        /* Destroy data associated with the window */
        DestroyDbgWndData(lpData);
    }

    /* Remove pointer to non-existing data */
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)NULL);

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
    LPDBGWNDDATA lpData = GetDbgWindowData(hwnd);
    
    /* Resize the status bar */
    SendDlgItemMessage(hwnd, IDC_DBGSTATUSBAR, WM_SIZE, wType,
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
    LPDBGWNDDATA lpData = GetDbgWindowData(hwnd);
    
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
HWND DbgWndCreate(
    HWND hwndParent
)
{
    LPDBGWNDDATA lpData;
    HWND hwndDebug;

    /* Allocate main window data structure */
    lpData = CreateDbgWndData();
    if(NULL == lpData)
    {
        return NULL;
    }
    
    /* Create main Window */
    hwndDebug = CreateDialogParam(
        g_hInstance,
        MAKEINTRESOURCE(IDD_DBG_WND),
        hwndParent,
        DialogProc,
        (LPARAM)lpData
    );
    
    /* Check if window was created*/
    if(NULL == hwndDebug)
    {
        DestroyDbgWndData(lpData);
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
VOID DbgWndLog(
    HWND hwndDebug,
    LPCSTR lpMessage
)
{
    LPDBGWNDDATA lpData = GetDbgWindowData(hwndDebug);
    INT iMsgLen;

    /* Get the message length */
    iMsgLen = lstrlenA(lpMessage);
    
    /* Make space for the message and new line characters  */
    if(EnsureBuffer(lpData, iMsgLen + 2))
    {
        /* If message length is saturated by buffer length - message is longer
         * than debug log buffer */
        iMsgLen = DBG_BUFFER_LEN - 3;
    }
    
    /* Add the message to the log */
    StringCchCopyA(
        lpData->strBuffer + lpData->iLogLength,
        iMsgLen + 1, /* There is always enough space for (truncated) message and
                      * terminating zero */
        lpMessage);
    
    /* Increase messa */
    lpData->iLogLength += iMsgLen;
    /* Add new line */
    lpData->strBuffer[lpData->iLogLength ++] = '\r';
    lpData->strBuffer[lpData->iLogLength ++] = '\n';
    lpData->strBuffer[lpData->iLogLength] = '\0';

    /* Update the window text */
    SetDlgItemTextA(hwndDebug, IDC_DBGCONSOLE, lpData->strBuffer);
    /* Move cursor to the end */
    SendDlgItemMessage(hwndDebug, IDC_DBGCONSOLE, EM_SETSEL,
        lpData->iLogLength - 1, lpData->iLogLength - 1);
    SendDlgItemMessage(hwndDebug, IDC_DBGCONSOLE, EM_SCROLLCARET, 0, 0);
}