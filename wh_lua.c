#include "wh_lua.h"
#include "resource.h"
#include "defs.h"
#include "main_wnd.h"


/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/

/**
 * @brief Name of the Lua table member representing @ref tagWHTIME::wHour
 * 
 */
#define LUA_HOUR_MEMBER "Hour"
/**
 * @brief Name of the Lua table member representing @ref tagWHTIME::wMinute
 * 
 */
#define LUA_MINUTE_MEMBER "Minute"

/**
 * @brief Function used to create time data table
 * 
 */
#define LUA_NEW_TIME_FCN "WhNewTime"

/**
 * @brief Function provided for debugging purposes
 * 
 */
#define LUA_ALERT_FCN "WhAlert"


/**
 * @brief Caption used for Lua-related error messages
 * 
 */
static LPCSTR g_lpMessageCaption = PROJECT_NAME ": Lua";


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
 * @brief Function to create time table in Lua
 * 
 * @param[in,out] lpLua
 * 
 * @return Number of outputs on Lua stack
 */
static int WhLuaNewTime(
    lua_State * lpLua
)
{
    WHTIME wht;
    
    /* Check arguments */
    if(2 != lua_gettop(lpLua) ||
        !lua_isnumber(lpLua, 1) || !lua_isnumber(lpLua, 2))
    {
        lua_pushstring(lpLua, "incorrect argument");
        lua_error(lpLua);
        return 0;
    }
    
    /* Get the values */
    wht.wHour = lua_tointeger(lpLua, 1);
    wht.wMinute = lua_tointeger(lpLua, 2);
    
    /* Push the time on the stack */
    WhLuaPushTime(lpLua, &wht);
    return 1;
}

/**
 * @brief Function to show custom Message Box
 * 
 * @param[in,out] lpLua
 * 
 * @return Number of outputs on Lua stack
 */
static int WhLuaAlert(
    lua_State * lpLua
)
{
    if(0 < lua_gettop(lpLua))
    {
        LPCSTR lpMsg;
        
        /* Get The string */
        lpMsg = lua_tostring(lpLua, -1);
        
        /* Show the message */
        MessageBoxA(g_hMainWnd, lpMsg, g_lpMessageCaption, MB_OK);
    }
    return 0;
}

/******************************************************************************/
/*                                Public                                      */
/******************************************************************************/

/******************************************************************************/
BOOL WhLuaInit(
    LPWH lpWh
)
{
    lpWh->lpLua = lua_newstate(WhLuaAllocator, (void *)lpWh);
    if(NULL == lpWh->lpLua)
        return FALSE;
    
    /* Initialize with no Lua code */
    lpWh->lpLuaCode = NULL;
    
    /* Register Lua Functions */
    lua_register(lpWh->lpLua, LUA_NEW_TIME_FCN, WhLuaNewTime);
    /*lua_register(lpWh->lpLua, LUA_ALERT_FCN, WhLuaAlert);*/

    return TRUE;
}

/******************************************************************************/
VOID WhLuaDestroy(
    LPWH lpWh
)
{
    if(NULL != lpWh->lpLua)
    {
        lua_close(lpWh->lpLua);
        lpWh->lpLua = NULL;
    }
    
    if(NULL != lpWh->lpLuaCode)
    {
        HeapFree(g_hHeap, 0, lpWh->lpLuaCode);
        lpWh->lpLuaCode = NULL;
    }
}

/******************************************************************************/
BOOL WhLuaSetCode(
    LPWH lpWh,
    LPSTR lpNewLuaCode
)
{
    /* Load string into the Lua state */
    if(0 != luaL_dostring(lpWh->lpLua, lpNewLuaCode))
        return FALSE;
    
    if(NULL != lpWh->lpLuaCode)
    {
        HeapFree(g_hHeap, 0, lpWh->lpLuaCode);
    }
    
    lpWh->lpLuaCode = lpNewLuaCode;
    
    return TRUE;
}

/******************************************************************************/
LPSTR WhLuaLoadDefaultCode(
    LPWH lpWh
)
{
    HRSRC hrscLua;
    HGLOBAL hLua;
    LPVOID lpOrigLua;
    DWORD dwLuaLength;
    LPSTR lpNewLua;
    
    /* Find the Lua code resource */
    hrscLua = FindResource(g_hInstance,
        MAKEINTRESOURCE(IDR_DEFAULT_LUA), RT_RCDATA);
    /* Was resource found */
    if(NULL == hrscLua)
        return NULL;

    /* Load the Lua code resource */
    hLua = LoadResource(NULL, hrscLua);
    /* Was resource loaded */
    if(NULL == hLua)
        return NULL;
    
    /* Get the Lua code */
    dwLuaLength = SizeofResource(g_hInstance, hrscLua);
    lpOrigLua = LockResource(hLua);
    /* Verify that resource was obtained */
    if(NULL == lpOrigLua || 0 == dwLuaLength)
        return NULL;
    
    /* Allocate buffer for Lua code string, add one byte for terminating zero */
    lpNewLua = HeapAlloc(g_hHeap, 0, dwLuaLength + 1);
    if(NULL == lpNewLua)
        return NULL;
    
    /* Copy Buffer */
    CopyMemory(lpNewLua, lpOrigLua, dwLuaLength);
    /* Add terminating zero */
    lpNewLua[dwLuaLength] = '\0';
           
    return lpNewLua;
}

/******************************************************************************/
VOID WhLuaErrorMessage(
    LPWH lpWh,
    HWND hwndParent
)
{
    LPCSTR lpMsg;
    
    /* Verify that there is a string (or an object convertible to string) on the
     * top of the stack */
    if(lua_isstring(lpWh->lpLua, -1))
    {
        /* Get the string */
        lpMsg = lua_tostring(lpWh->lpLua, -1);

        if(NULL != lpMsg)
        {
            MessageBoxA(hwndParent, lpMsg, g_lpMessageCaption,
                MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
            /* Pop the error message from the stack */
            lua_pop(lpWh->lpLua, 1);
        }
    }
}

/******************************************************************************/
VOID WhLuaPushTime(
    lua_State * lpLua,
    const LPWHTIME lpTime
)
{
    /* Create table with two elements */
    lua_createtable(lpLua, 0, 2);
    
    /* Set the fields */
    lua_pushinteger(lpLua, lpTime->wHour);
    lua_setfield(lpLua, -2, LUA_HOUR_MEMBER);
    lua_pushinteger(lpLua, lpTime->wMinute);
    lua_setfield(lpLua, -2, LUA_MINUTE_MEMBER);
}

/******************************************************************************/
BOOL WhLuaPopTime(
    lua_State * lpLua,
    LPWHTIME lpTime
)
{
    /* Verify the type */
    if(!lua_istable(lpLua, -1))
        return FALSE;
    
    /* Get the fields */
    lua_getfield(lpLua, -1, LUA_HOUR_MEMBER);
    lpTime->wHour = lua_tointeger(lpLua, -1);
    lua_pop(lpLua, 1);
    lua_getfield(lpLua, -1, LUA_MINUTE_MEMBER);
    lpTime->wMinute = lua_tointeger(lpLua, -1);
        
    /* Pop the last integer and the table */
    lua_pop(lpLua, 2);
    
    return TRUE;
}

/******************************************************************************/
BOOL WhLuaPopColor(
    lua_State * lpLua,
    LPCOLORREF lpcrColor
)
{
    /* Verify the type */
    if(!lua_isnumber(lpLua, -1))
        return FALSE;
    
    /* Get the color */
    *lpcrColor = (COLORREF)lua_tointeger(lpLua, -1);
    
    /* Pop the value */
    lua_pop(lpLua, 1);
    
    return TRUE;
}