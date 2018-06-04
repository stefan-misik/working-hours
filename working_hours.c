#include "working_hours.h"

#define LUA_HEAP_INITIAL_SIZE (1*1024)
#define LUA_HEAP_MAX_SIZE (1024*1024)

/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/

/**
 * @brief Structure used by the Lua string reader
 * 
 */
typedef struct tagLUAREADSTR
{
    size_t nLength;     /**< The length of the Lua string */
    size_t nPosition;   /**< The current position in the Lua string */
    const char * lpStr; /**< The Lua string */
} LUAREADSTR, *LPLUAREADSTR;

/**
 * @brief Function used by the Lua language to allocate/reallocate/free memory
 * 
 * @param lpData Passed pointer
 * @param lpPtr Old pointer to memory, or NULL when allocating new memory
 * @param nOldSize Old size of allocated memory in bytes
 * @param nNewSize Requested new size of the memory in bytes
 * 
 * @return New pointer to memory, or NULL when freeing memory
 */
static void * WhLuaAllocator(
    void * lpData, 
    void * lpPtr,
    size_t nOldSize,
    size_t nNewSize
)
{
    if(0 == nNewSize)
    {
        /* HeapFree has undefined behavior for null pointers */
        if(NULL != lpPtr)
        {
            HeapFree(g_hHeap, 0, lpPtr);
        }
        return NULL;
    }
    else
    {
        LPVOID lpRet;
        
        if(NULL == lpPtr)
        {
            lpRet = HeapAlloc(g_hHeap, 0, nNewSize);
        }
        else
        {
            lpRet = HeapReAlloc(g_hHeap, 0, lpPtr, nNewSize);
        }        
        return lpRet;
    }
}

/**
 * @brief Function used to read the whole string at once
 * 
 * @param lpLua Lua state 
 * @param[in,out] lpData Passed pointer
 * @param lpSize
 * @return 
 */
static const char * WhLuaStringReared(
    lua_State * lpLua,
    void * lpData,
    size_t * lpSize
)
{
    const char * lpRet = NULL;
    
    if(((LPLUAREADSTR)lpData)->nPosition < ((LPLUAREADSTR)lpData)->nLength)
    {
        /* Calculate the size */
        *lpSize = (((LPLUAREADSTR)lpData)->nLength -
            ((LPLUAREADSTR)lpData)->nPosition);
        /* Read the string */
        lpRet = ((LPLUAREADSTR)lpData)->lpStr + 
            ((LPLUAREADSTR)lpData)->nPosition;
        /* Increment the position */
        ((LPLUAREADSTR)lpData)->nPosition += (*lpSize);
    }
    else
        *lpSize = 0;
    
    return lpRet;
}

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
    lpWh->lpLua = lua_newstate(WhLuaAllocator, (void *)lpWh);
    if(NULL == lpWh->lpLua)
        return FALSE;
    
    return TRUE;
}


/******************************************************************************/
VOID WhDestroy(
    LPWH lpWh
)
{
    lua_close(lpWh->lpLua);
    
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
