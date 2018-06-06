#include "wh_lua.h"
#include "resource.h"
#include "defs.h"


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

/**
 * @brief Caption used for Lua-related error messages
 * 
 */
static LPCSTR lpErrorCaption = PROJECT_NAME ": Lua Error";


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
            MessageBoxA(hwndParent, lpMsg, lpErrorCaption,
                MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
            /* Pop the error message from the stack */
            lua_pop(lpWh->lpLua, 1);
        }
    }
}