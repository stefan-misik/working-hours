#include "working_hours.h"

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
    const LPSYSTEMTIME lpSystime
)
{
    lpWht->wHour = lpSystime->wHour;
    lpWht->wMinute = lpSystime->wMinute;
    
    return TRUE;
}

/******************************************************************************/
BOOL WhWhtToSystime(
    LPSYSTEMTIME lpSystime,
    const LPWHTIME lpWht
)
{
    ZeroMemory(lpSystime, sizeof(SYSTEMTIME));
    lpSystime->wHour = lpWht->wHour;
    lpSystime->wMinute = lpWht->wMinute;

    return TRUE;
}

/******************************************************************************/
BOOL WhInit(
    LPWH lpWh
)
{
    /* Create new Lua state */
    if(!WhLuaInit(lpWh))
        return FALSE;
    
    return TRUE;
}


/******************************************************************************/
VOID WhDestroy(
    LPWH lpWh
)
{
    WhLuaDestroy(lpWh);
}

/******************************************************************************/
BOOL WhCalculate(
    LPWH lpWh,
    const LPWHTIME lpwhtArrival,
    const LPWHTIME lpwhtNow,
    LPWHTIME lpwhtWorked,
    LPCOLORREF lpcrColor
)
{
    /* Push Calculate Lua function on the stack */
    lua_getglobal(lpWh->lpLua, LUA_CALCULATE_FCN);
    
    /* Push input parameters onto Lua stack */
    WhLuaPushTime(lpWh->lpLua, lpwhtArrival);
    WhLuaPushTime(lpWh->lpLua, lpwhtNow);
    
    /* Call the Lua function */
    if(0 != lua_pcall(lpWh->lpLua, 2, 2, 0))
    {
        WhLuaErrorMessage(lpWh, NULL);
        return FALSE;
    }

    /* Get the Color */
    WhLuaPopColor(lpWh->lpLua, lpcrColor);
    /* Get the the time worked result */
    WhLuaPopTime(lpWh->lpLua, lpwhtWorked);

    return TRUE;
}



/******************************************************************************/
BOOL WhLeaveTime(
    LPWH lpWh,
    const LPWHTIME lpwhtArrival,
    LPWHTIME lpwhtLeave
)
{
    /* Push Leave Time Lua function on the stack */
    lua_getglobal(lpWh->lpLua, LUA_LEAVETIME_FCN);
    
    /* Push input parameters onto Lua stack */
    WhLuaPushTime(lpWh->lpLua, lpwhtArrival);
    
    /* Call the Lua function */
    if(0 != lua_pcall(lpWh->lpLua, 1, 1, 0))
    {
        WhLuaErrorMessage(lpWh, NULL);
        return FALSE;
    }

    /* Get the the time worked result */
    WhLuaPopTime(lpWh->lpLua, lpwhtLeave);

    return TRUE;
}
