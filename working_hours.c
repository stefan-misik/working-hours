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
    INT iMinutes, iMinutesNow;

	iMinutesNow = lpwhtNow->wMinute + (60 * lpwhtNow->wHour);
    iMinutes = iMinutesNow -
		(lpwhtArrival->wMinute + (60 * lpwhtArrival->wHour));
    
    /* Subtract 5 minutes form beginning */
    iMinutes -= 5;
    
	/* Subtract 30 minutes lunch break after 6 hours */
    if(iMinutes > (6*60 + 30))
        iMinutes -= 30;
    else if (iMinutes > (6*60))
        iMinutes = 6*60;
    
    /* Subtract 5 minutes from end */
    iMinutes -= 5;
    
    /* Make sure the time spent working is not negative */
    if(iMinutes < 0)
	{
        iMinutes = 0;
	}

	/* Assign counter color */
	if(iMinutesNow < (14*60 + 30))
	{
		/* RED - Before and of obligatory period */
		*lpcrColor = RGB(237, 28, 36);
	}
	else if (iMinutes < (8*60))
	{
		/* ORANGE - After end of obligatory period, before 8 hours ofwork */
		*lpcrColor = RGB(255, 127, 39);
	}
	else
	{
		/* GREEN - After 8 hours of work and after obligatory period */
		*lpcrColor = RGB(34, 177, 76);
	}
    
    lpwhtWorked->wMinute = iMinutes % 60;
    lpwhtWorked->wHour = iMinutes / 60;

    
    return TRUE;
}



/******************************************************************************/
BOOL WhLeaveTime(
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
    
    return TRUE;
}
