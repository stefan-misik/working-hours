#ifndef MAIN_WND_H
#define MAIN_WND_H

#include "win_common.h"

/**
 * @brief The handle to the main window
 * 
 */
extern HWND g_hMainWnd;

/**
 * @brief Creates and accordingly shows main application window
 * 
 * @param nCmdShow How to show the main window
 * @return FALSE on failure
 */
BOOL CreateMainWindow(
    int nCmdShow
);


#endif /* MAIN_WND_H */

