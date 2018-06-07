#ifndef WORKING_HOURS_H
#define WORKING_HOURS_H

#include "win_common.h"
#include "wh_lua.h"




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
 * @param[in,out] lpWhLua Working hours Lua state
 * @param[in] lpWhtArrival Arrival time
 * @param[in] lpwhtNow Current time
 * @param[out] lpwhtWorked Pointer which obtains the time spent working
 * @param[out] lpcrColor Pointer which can optionally obtain informative color
 * 
 * @return FALSE on failure
 */
BOOL WhCalculate(
    LPWHLUA lpWhLua,
    const LPWHTIME lpwhtArrival,
    const LPWHTIME lpwhtNow,
    LPWHTIME lpwhtWorked,
    LPCOLORREF lpcrColor
);

/**
 * @brief Calculate leave time
 * 
 * 
 * @param[in,out] lpWhLua Working hours Lua state
 * @param[in] lpwhtArrival
 * @param[out] lpwhtLeaveTime
 * 
 * @return FALSE on failure
 */
BOOL WhLeaveTime(
    LPWHLUA lpWhLua,
    const LPWHTIME lpwhtArrival,
    LPWHTIME lpwhtLeave
);

#endif /* WORKING_HOURS_H */
