#include "tray_icon.h"
#include <strsafe.h>


/******************************************************************************/
/*                                Public                                      */
/******************************************************************************/

/******************************************************************************/
BOOL TrayIconAdd(
    HWND hWnd,
    UINT uID,
    UINT uCallbackMsg,
    HICON hTrayIcon
)
{
    NOTIFYICONDATA  nid;

    memset(&nid, 0, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.uVersion = NOTIFYICON_VERSION;
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE;
    nid.uCallbackMessage = uCallbackMsg;
    nid.hIcon = hTrayIcon;

    return Shell_NotifyIcon(NIM_ADD, &nid);
}

/******************************************************************************/
BOOL TrayUpdateText(
    HWND hWnd,
    UINT uID,
    LPTSTR lptstrNewText    
)
{
    NOTIFYICONDATA  nid;

    memset(&nid, 0, sizeof(nid));
    
    nid.cbSize = sizeof(nid);
    nid.uVersion = NOTIFYICON_VERSION;
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_TIP;
    
    /* Copy the new string */
    if(FAILED(StringCchCopy(nid.szTip, sizeof(nid.szTip) / sizeof(TCHAR),
        lptstrNewText)))
    {
        return FALSE;
    }
    
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

/******************************************************************************/
BOOL TrayIconRemove(
    HWND hWnd,
    UINT uID
)
{
    NOTIFYICONDATA  nid;

    memset(&nid, 0, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = uID;

    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

/******************************************************************************/
BOOL TrayIconShowPopupMenu(
    HWND hwndParent,
    HMENU hMenu
)
{
    POINT pt;

	/* Get cursor pos */
    if(FALSE == GetCursorPos(&pt))
        return FALSE;

    /* Window receiving the commands needs to be in focus, even if hidden */
    SetForegroundWindow(hwndParent);

    /* Show popum menu */
    return TrackPopupMenuEx(
        GetSubMenu(hMenu, 0),
        TPM_LEFTBUTTON,
        pt.x, pt.y,
        hwndParent,
        NULL
    );
}
