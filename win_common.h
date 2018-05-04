#ifndef COMMON_H
#define	COMMON_H

#undef _WIN32_WINNT
#undef _WIN32_IE
#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0900
#ifndef _UNICODE
    #define _UNICODE
#endif
#ifndef UNICODE
    #define UNICODE
#endif
#define OEMRESOURCE

#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>

extern HINSTANCE g_hInstance;
extern HANDLE g_hHeap;


#endif	/* COMMON_H */