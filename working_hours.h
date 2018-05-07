#ifndef WORKING_HOURS_H
#define WORKING_HOURS_H

#include "win_common.h"

typedef struct tagWHTIME
{
    WORD wHour;
    WORD wMinute;
} WHTIME, *LPWHTIME;
#endif /* WORKING_HOURS_H */


/**
 * @brief Convert windows system time into working hours time format
 * 
 * @param lpWht Working hours time to obtain the converted value
 * @param lpSystime System time structure containing time value to convert
 * 
 * @return Always TRUE 
 */
BOOL WhSystimeToWht(
    LPWHTIME lpWht,
    const LPSYSTEMTIME lpSystime
);

/**
 * @brief Convert working hours time format into windows system time
 * 
 * @param lpSystime System time structure to obtain the converted value
 * @param lpWht Working hours time containing the time value to convert
 * 
 * @return Always TRUE 
 */
BOOL WhWhtToSystime(
    LPSYSTEMTIME lpSystime,
    const LPWHTIME lpWht
);

/**
 * @brief Calculate the time spent working
 * 
 * @param lpWhtArrival Arrival time
 * @param lpwhtNow Current time
 * @param lpwhtWorked Pointer which obtains the time spent working
 * @param lpcrColor Pointer which can optionally obtain informative color
 * 
 * @return Always TRUE 
 */
BOOL WhCalculate(
    const LPWHTIME lpwhtArrival,
    const LPWHTIME lpwhtNow,
    LPWHTIME lpwhtWorked,
    LPCOLORREF lpcrColor
);