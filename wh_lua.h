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
typedef struct tagWH
{
    lua_State * lpLua;  /**< Lua interpreter state */
    LPSTR lpLuaCode;    /**< Current Lua Code */
} WH, *LPWH;


/**
 * @brief Initialize Lua sate in working hours state
 * 
 * @param[in,out] lpWh Working hours state
 * 
 * @return FALSE on failure
 */
BOOL WhLuaInit(
    LPWH lpWh
);

/**
 * @brief Destroy Lua state in working hours state
 * 
 * @param[in,out] lpWh Working hours state
 */
VOID WhLuaDestroy(
    LPWH lpWh
);

/**
 * @brief Set new Lua code
 * 
 * @param[in,out] lpWh Working hours state
 * @param lpNewLuaCode Pointer to buffer containing the string with new Lua code
 */
VOID WhSetLuaCode(
    LPWH lpWh,
    LPSTR lpNewLuaCode
);

/**
 * @brief Load the default Lua code from resources
 * 
 * @param[in,out] lpWh Working hours state
 * 
 * @return FALSE on failure
 */
BOOL WhLoadDefaultLuaCode(
    LPWH lpWh
);


#endif /* WH_LUA_H */

