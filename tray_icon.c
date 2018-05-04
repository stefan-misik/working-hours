#include "tray_icon.h"

/******************************************************************************/
/*                                Public                                      */
/******************************************************************************/

/******************************************************************************/
BOOL TrayIconAdd(
    HWND hwnd,
    UINT uID,
    UINT uCallbackMsg,
    HICON hTrayIcon
)
{
    NOTIFYICONDATA  nid;

    memset(&nid, 0, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.uVersion = NOTIFYICON_VERSION;
    nid.hWnd = hwnd;
    nid.uID = uID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = uCallbackMsg;
    nid.hIcon = hTrayIcon;

    GetWindowText(hwnd, nid.szTip, sizeof(nid.szTip) / sizeof(TCHAR));

    return Shell_NotifyIcon(NIM_ADD, &nid);
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
