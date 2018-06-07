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
    LPSTR lpLuaCode;    /**< Current Lua Code */
    HWND hwndParent;    /**< Parent window used for error messages */
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
 * @return FALSE on failure
 */
BOOL WhLuaInit(
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
 * @brief Set new Lua code and loads it into the Lua state
 * 
 * @param[in,out] lpWhLua Working hours state
 * @param lpNewLuaCode Pointer to buffer containing the string with new Lua code
 * 
 * @return FALSE on failure
 */
BOOL WhLuaSetCode(
    LPWHLUA lpWhLua,
    LPSTR lpNewLuaCode
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
LPSTR WhLuaLoadDefaultCode(VOID);

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

#endif /* WH_LUA_H */

