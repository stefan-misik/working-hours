#include "win_common.h"
#include "resource.h"
#include "defs.h"
#include "main_wnd.h"
#include "accelerators.h"

/**
 * @brief Window handle to receive the accelerator generated messages
 * 
 * NULL to turn off accelerator.
 */
static HWND g_hWndAcc;

/**
 * @brief Accelerator table handle
 */
static HACCEL g_hAccel;

HINSTANCE g_hInstance;
HANDLE g_hHeap;

/******************************************************************************/
VOID AccRegisterWindow(
    HWND hwnd
)
{
    g_hWndAcc = (NULL == g_hAccel) ? NULL : hwnd;
}

/******************************************************************************/
INT WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
	)
{
    MSG msg;
    INITCOMMONCONTROLSEX icc;
    
    /* Initialize common controls */
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES |
            ICC_LISTVIEW_CLASSES;
    if(!InitCommonControlsEx(&icc))
        return -1;

    /* Store main instance handle */
    g_hInstance = hInstance;  

    /* Get Process heap */
    g_hHeap = GetProcessHeap();

    /* Create the main window */
    if(!CreateMainWindow(nCmdShow))
    {
        LPTSTR lpMsg;
        
        /* Load error message */
        LoadString(g_hInstance, IDS_MAIN_ERROR, (LPTSTR)(&lpMsg), 0);
        
        /* Show error message */
        MessageBox(NULL, lpMsg, TEXT(PROJECT_NAME), MB_OK | MB_ICONHAND);
        return 1;
    }
    
    /* Load the accelerator table */
    g_hAccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACC));
    g_hWndAcc = NULL;
    
    /* Enter the message loop */
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        /* Translate dialog messages */
        if(IsDialogMessage(g_hMainWnd, &msg))
            continue;
        
        /* Translate accelerator messages */
        if(NULL != g_hWndAcc && TranslateAccelerator(g_hWndAcc, g_hAccel, &msg))
            continue;
        
        /* Translate and dispatch messages */
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	ExitProcess((UINT)(msg.wParam));

   return (INT)(msg.wParam);
}
