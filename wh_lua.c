#include "wh_lua.h"
#include "resource.h"


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
VOID WhSetLuaCode(
    LPWH lpWh,
    LPSTR lpNewLuaCode
)
{
    if(NULL != lpWh->lpLuaCode)
    {
        HeapFree(g_hHeap, 0, lpWh->lpLuaCode);
    }
    
    lpWh->lpLuaCode = lpNewLuaCode;
}

/******************************************************************************/
BOOL WhLoadDefaultLuaCode(
    LPWH lpWh
)
{
    HRSRC hrscLua;
    HGLOBAL hLua;
    LPVOID lpOrigLua;
    DWORD dwLuaLength;
    LPSTR lpNewLua;
    
    /* Get Lua resource info */
    hrscLua = FindResource(g_hInstance,
        MAKEINTRESOURCE(IDR_DEFAULT_LUA), RT_RCDATA);
    hLua = LoadResource(NULL, hrscLua);
    dwLuaLength = SizeofResource(g_hInstance, hrscLua);
    lpOrigLua = LockResource(hLua);
    
    /* Allocate buffer for Lua code string, add one byte for terminating zero */
    lpNewLua = HeapAlloc(g_hHeap, 0, dwLuaLength + 1);
    if(NULL == lpNewLua)
        return FALSE;
    
    /* Copy Buffer */
    CopyMemory(lpNewLua, lpOrigLua, dwLuaLength);
    /* Add terminating zero */
    lpNewLua[dwLuaLength] = '\0';
    
    /* Set the code */
    WhSetLuaCode(lpWh, lpNewLua);
    
    return TRUE;
}