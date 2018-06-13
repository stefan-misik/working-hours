#ifndef LUA_EDIT_H
#define LUA_EDIT_H

#include "win_common.h"

#define WM_LEWNDOPENCLOSE (WM_USER + 2)

/**
 * @brief Create the Lua editor window
 * 
 * @param hwndParent Handle to the parent window
 * 
 * @return Handle to newly created debug window or NULL on failure
 */
HWND LeWndCreate(
    HWND hwndParent
);

#endif /* LUA_EDIT_H */

