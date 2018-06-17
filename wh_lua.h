#ifndef WH_LUA_H
#define WH_LUA_H

#include "win_common.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/**
 * @brief Structure holding state information for working hours calculation
 * 
 */
typedef struct tagWHLUA
{
    lua_State * lpLua;  /**< Lua interpreter state */
    HWND hwndParent;    /**< Parent window used for error messages */
    HWND hwndDebugWnd;  /**< Window used for printing the Lua debug messages,
                         *   can be NULL */
} WHLUA, *LPWHLUA;

/**
 * @brief Time data structure
 * 
 */
typedef struct tagWHTIME
{
    WORD wHour;
    WORD wMinute;
} WHTIME, *LPWHTIME;

/**
 * @brief Initialize Lua sate in working hours state
 * 
 * @param[out] lpWhLua Working hours state
 * 
 * @warning Lua state is not yer ready after calling this function, it needs to
 *          be created by calling @ref WhLuaReset()
 */
VOID WhLuaInit(
    LPWHLUA lpWhLua
);

/**
 * @brief Destroy Lua state in working hours state
 * 
 * @param[in,out] lpWhLua Working hours Lua state
 */
VOID WhLuaDestroy(
    LPWHLUA lpWhLua
);

/**
 * @brief (Re)create the Lua state
 * 
 * @param[in,out] lpWhLua Working hours Lua state
 * 
 * @return False on Failure
 */
BOOL WhLuaReset(
    LPWHLUA lpWhLua
);

/**
 * @brief Set the parent window for Lua-related error message-boxes
 * 
 * @param[out] lpWhLua Working hours Lua state
 * @param hwndParent New Parent window
 */
VOID WhLuaSetErrorParentWnd(
    LPWHLUA lpWhLua,
    HWND hwndParent
);

/**
 * @brief Set the debug window for displaying Lua Messages
 * 
 * @param[out] lpWhLua Working hours Lua state
 * @param hwndDbg Debug window handle, NULL do disable debug messages
 */
VOID WhLuaSetDebugWnd(
    LPWHLUA lpWhLua,
    HWND hwndDbg
);

/**
 * @brief Print string from the top of Lua stack as an error message
 * 
 * @param[in] lpWhLua Working hours state
 */
VOID WhLuaErrorMessage(
    LPWHLUA lpWhLua
);

/**
 * @brief Push the time structure on Lua stack
 * 
 * @param[in,out] lpLua Lua state
 * @param[in] lpTime Pointer to time value to be pushed on the Lua stack
 */
VOID WhLuaPushTime(
    lua_State * lpLua,
    const LPWHTIME lpTime
);

/**
 * @brief Pop the time structure from Lua stack
 * 
 * @param[in,out] lpLua Lua state
 * @param[out] lpTime Pointer to time value which shall receive popped values
 * @param iIndex Position in Lua stack
 * 
 * @return FALSE on Failure
 * 
 * @warning Function will not pop a value when fails
 */
BOOL WhLuaToTime(
    lua_State * lpLua,
    LPWHTIME lpTime,
    INT iIndex
);

/**
 * @brief Pop the color from Lua stack
 * 
 * @param[in,out] lpLua Lua state
 * @param[out] lpTime Pointer to variable which shall receive popped value
 * @param iIndex Position in Lua stack
 * 
 * @return FALSE on Failure
 * 
 * @warning Function will not pop a value when fails
 */
BOOL WhLuaToColor(
    lua_State * lpLua,
    LPCOLORREF lpcrColor,
    INT iIndex
);

/**
 * @brief Load the Lua code from a text file
 * 
 * @param[in] lpFile Path to a file to be loaded
 * 
 * @return Allocated buffer with the loaded Lua code or NULL on failure
 * 
 * @warning The returned buffer needs to be freed with HeapFree() using the
 *          default process heap GetProcessHeap()
 */
LPSTR WhLuaLoadCode(
    LPCTSTR lpFile
);

/**
 * @brief Load the default Lua code from resources
 * 
 * @return Allocated buffer with the default Lua code or NULL on failure
 * 
 * @warning The returned buffer needs to be freed with HeapFree() using the
 *          default process heap GetProcessHeap()
 */
LPSTR WhLuaLoadDefaultCode(
    VOID
);

/**
 * @brief Load specified string into Lua
 * 
 * @param[in,out] lpWhLua Working hours Lua state
 * @param[in] lpFileName Name of the file containing the Lua code
 * @param[in] lpLuaCode Lua code to be loaded into the Lua state
 * 
 * @return FALSE on failure
 */
BOOL WhLuaDoString(
    LPWHLUA lpWhLua,
    LPCSTR lpFileName,
    LPCSTR lpLuaCode
);

#endif /* WH_LUA_H */

