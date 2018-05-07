#include "working_hours.h"

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
BOOL WhCalculate(
    const LPWHTIME lpwhtArrival,
    const LPWHTIME lpwhtNow,
    LPWHTIME lpwhtWorked,
    LPCOLORREF lpcrColor
)
{
    INT iMinutes;
        
    iMinutes = lpwhtNow->wMinute + (60 * lpwhtNow->wHour);
    iMinutes -= lpwhtArrival->wMinute + (60 * lpwhtArrival->wHour);
    
    /* Subtract 5 minutes form beginning */
    iMinutes -= 5;
    
    if(iMinutes > 390)
        iMinutes -= 30;
    else if (iMinutes > 360)
        iMinutes = 360;
    
    /* Subtract 5 minutes from end */
    iMinutes -= 5;
    
    /* Make sure the time spent working is not negative */
    if(iMinutes < 0)
        iMinutes = 0;
    
    lpwhtWorked->wMinute = iMinutes % 60;
    lpwhtWorked->wHour = iMinutes / 60;

    
    return TRUE;
}