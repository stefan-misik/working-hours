#include "working_hours.h"
#include "main_wnd.h"

/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/

/**
 * @brief Lua function calculating the working hours
 * 
 */
#define LUA_CALCULATE_FCN "Calculate"

/**
 * @brief Lua function calculating the leave time
 * 
 */
#define LUA_LEAVETIME_FCN "LeaveTime"

/******************************************************************************/
/*                                Public                                      */
/******************************************************************************/

/******************************************************************************/
BOOL WhSystimeToWht(
    LPWHTIME lpWht,
    const SYSTEMTIME * lpSystime
)
{
    lpWht->wHour = lpSystime->wHour;
    lpWht->wMinute = lpSystime->wMinute;
    
    return TRUE;
}

/******************************************************************************/
BOOL WhWhtToSystime(
    LPSYSTEMTIME lpSystime,
    LPCWHTIME lpWht
)
{
    ZeroMemory(lpSystime, sizeof(SYSTEMTIME));
    lpSystime->wHour = lpWht->wHour;
    lpSystime->wMinute = lpWht->wMinute;

    return TRUE;
}

/******************************************************************************/
BOOL WhCalculate(
    LPWHLUA lpWhLua,
    LPCWHTIME lpwhtArrival,
    LPCWHTIME lpwhtNow,
    DWORD wPauseTime,
    LPWHTIME lpwhtWorked,
    LPCOLORREF lpcrColor
)
{
    BOOL bRet = TRUE;
    /* Push Calculate Lua function on the stack */
    lua_getglobal(lpWhLua->lpLua, LUA_CALCULATE_FCN);
    
    /* Push input parameters onto Lua stack */
    WhLuaPushTime(lpWhLua->lpLua, lpwhtArrival);
    WhLuaPushTime(lpWhLua->lpLua, lpwhtNow);
    
    /* Call the Lua function */
    if(0 != lua_pcall(lpWhLua->lpLua, 2, 2, 0))
    {
        WhLuaErrorMessage(lpWhLua);
        return FALSE;
    }

    /* Get the Color */
    bRet = bRet && WhLuaToColor(lpWhLua->lpLua, lpcrColor, -1);

    /* Get the the time worked result */
    bRet = bRet && WhLuaToTime(lpWhLua->lpLua, lpwhtWorked, -2);
    
    /* Pop the stack */
    lua_pop(lpWhLua->lpLua, 2);

    return bRet;
}



/******************************************************************************/
BOOL WhLeaveTime(
    LPWHLUA lpWhLua,
    LPCWHTIME lpwhtArrival,
    DWORD wPauseTime,
    LPWHTIME lpwhtLeave
)
{
    BOOL bRet = TRUE;
    
    /* Push Leave Time Lua function on the stack */
    lua_getglobal(lpWhLua->lpLua, LUA_LEAVETIME_FCN);
    
    /* Push input parameters onto Lua stack */
    WhLuaPushTime(lpWhLua->lpLua, lpwhtArrival);
    
    /* Call the Lua function */
    if(0 != lua_pcall(lpWhLua->lpLua, 1, 1, 0))
    {
        WhLuaErrorMessage(lpWhLua);
        return FALSE;
    }

    /* Get the the time worked result */
    bRet = bRet && WhLuaToTime(lpWhLua->lpLua, lpwhtLeave, -1);
    
    /* Pop the stack */
    lua_pop(lpWhLua->lpLua, 1);    

    return TRUE;
}
