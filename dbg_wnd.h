#ifndef DEBUG_WND_H
#define DEBUG_WND_H

#include "win_common.h"

#define WM_DBGWNDOPENCLOSE (WM_USER + 1)


/**
 * @brief Create the show the debug window
 * 
 * @param hwndParent Handle to the parent window
 * 
 * @return Handle to newly created debug window or NULL on failure
 */
HWND DbgWndCreate(
    HWND hwndParent
);

/**
 * @brief Log a message into a debug console
 * 
 * @param hwndDebug Debug window handle
 * @param[in] lpMessage Message to be logged
 */
VOID DbgWndLog(
    HWND hwndDebug,
    LPCSTR lpMessage
);

#endif /* DEBUG_WND_H */

