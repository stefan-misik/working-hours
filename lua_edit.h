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

/**
 * @brief Load code into the editor
 * 
 * @param hwndLuaEdit Lua editor window handle
 * @param[in] lpFilename Lua source file name
 * @param[in] lpLuaCode Lua code
 */
VOID LeWndSetCode(
    HWND hwndLuaEdit,
    LPCSTR lpFilename,
    LPCSTR lpLuaCode
);

#endif /* LUA_EDIT_H */

