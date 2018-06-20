#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include "win_common.h"

/**
 * @brief Add a tray icon
 * 
 * @param hWnd Window owning the icon
 * @param uID ID of the icon
 * @param uCallbackMsg Message sent by the tray icon to the owning window
 * @param hTrayIcon Icon to be shown in the tray
 * 
 * @return FALSE on failure 
 */
BOOL TrayIconAdd(
	HWND hWnd,
	UINT uID,
	UINT uCallbackMsg,
    HICON hTrayIcon
);

/**
 * @brief Update balloon text of the notification icon
 * 
 * @param hWnd Window owning the icon
 * @param uID ID of the icon
 * @param lptstrNewText New text
 *
 * @return FALSE on failure 
 */
BOOL TrayUpdateText(
    HWND hWnd,
    UINT uID,
    LPTSTR lptstrNewText    
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

/**
 * @brief get the message ID of the message sent on creation of taskbar
 * 
 * @return Message number
 */
UINT TrayIconTaskbarCreatedMessage(
    VOID
);

#endif /* TRAY_ICON_H */

