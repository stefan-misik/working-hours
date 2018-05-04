#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include "win_common.h"

/**
 * @brief Add a tray icon
 * 
 * @param hWnd Window owning the icon
 * @param uID ID of the icon
 * @param uCallbackMsg ??
 * @param hTrayIcon Icon to be shown in the tray
 * 
 * @return FALSE on failure 
 */
BOOL TrayIconAdd(
	HWND hwnd,
	UINT uID,
	UINT uCallbackMsg,
    HICON hTrayIcon
);

/**
 * @brief Remove the tray icon
 * 
 * @param hWnd Window owning the icon
 * @param uID ID of the icon
 * 
 * @return FALSE on failure 
 */
BOOL TrayIconRemove(
	HWND hWnd,
	UINT uID
);

/**
 * @brief Show a popup menu near the place where mouse is
 * 
 * @param hwndParent Window which receives menu command message
 * @param hMenu Menu to be shown
 * 
 * @return 0 on failure or cancellation by use
 */
BOOL TrayIconShowPopupMenu(
    HWND hwndParent,
    HMENU hMenu
);

#endif /* TRAY_ICON_H */

