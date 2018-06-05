#ifndef WORKING_HOURS_H
#define WORKING_HOURS_H

#include "win_common.h"
#include "wh_lua.h"


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
 * @brief Initialize working hours state
 * 
 * @param lpWh Working hours state to be initialized
 * 
 * @return FALSE on failure, otherwise success
 */
BOOL WhInit(
    LPWH lpWh
);

/**
 * @brief Destroy working hours state
 * 
 * @param lpWh Working hours to be destroyed
 */
VOID WhDestroy(
    LPWH lpWh
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
    LPWH lpWh,
    const LPWHTIME lpwhtArrival,
    const LPWHTIME lpwhtNow,
    LPWHTIME lpwhtWorked,
    LPCOLORREF lpcrColor
);

/**
 * @brief Calculate leave time
 * 
 * Time 8 hours from arrival time
 * 
 * @param lpwhtArrival
 * @param lpwhtLeaveTime
 * @return 
 */
BOOL WhLeaveTime(
    LPWH lpWh,
    const LPWHTIME lpwhtArrival,
    LPWHTIME lpwhtLeave
);

#endif /* WORKING_HOURS_H */
