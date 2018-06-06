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
    // INT iMinutes, iMinutesNow;
    
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

//    iMinutesNow = lpwhtNow->wMinute + (60 * lpwhtNow->wHour);
//    iMinutes = iMinutesNow -
//		(lpwhtArrival->wMinute + (60 * lpwhtArrival->wHour));
//    
//    /* Subtract 5 minutes form beginning */
//    iMinutes -= 5;
//    
//	/* Subtract 30 minutes lunch break after 6 hours */
//    if(iMinutes > (6*60 + 30))
//        iMinutes -= 30;
//    else if (iMinutes > (6*60))
//        iMinutes = 6*60;
//    
//    /* Subtract 5 minutes from end */
//    iMinutes -= 5;
//    
//    /* Make sure the time spent working is not negative */
//    if(iMinutes < 0)
//    {
//        iMinutes = 0;
//    }
//
//    /* Assign counter color */
//    if(iMinutesNow < (14*60 + 30))
//    {
//        /* RED - Before and of obligatory period */
//        *lpcrColor = RGB(237, 28, 36);
//    }
//    else if (iMinutes < (8*60))
//    {
//        /* ORANGE - After end of obligatory period, before 8 hours ofwork */
//        *lpcrColor = RGB(255, 127, 39);
//    }
//    else
//    {
//        /* GREEN - After 8 hours of work and after obligatory period */
//        *lpcrColor = RGB(34, 177, 76);
//    }
//    
//    lpwhtWorked->wMinute = iMinutes % 60;
//    lpwhtWorked->wHour = iMinutes / 60;

    return TRUE;
}



/******************************************************************************/
BOOL WhLeaveTime(
    LPWH lpWh,
    const LPWHTIME lpwhtArrival,
    LPWHTIME lpwhtLeave
)
{
    INT iMinutes;

    iMinutes = lpwhtArrival->wMinute + (60 * lpwhtArrival->wHour);

    /* Leave in 8 hours, 30 minutes for launch break, and twice 5 minutes */
    iMinutes += (8 * 60) + 30 + 5 + 5;
    
    lpwhtLeave->wMinute = iMinutes % 60;
    lpwhtLeave->wHour = iMinutes / 60;

    /* Fix-up hours */
    if(lpwhtLeave->wHour > 23)
	lpwhtLeave->wHour %= 24;
    
    return TRUE;
}
