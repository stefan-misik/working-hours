#include "dialog_resize.h"

/******************************************************************************/
/*                               Private                                      */
/******************************************************************************/


/******************************************************************************/
/*                                Public                                      */
/******************************************************************************/

/******************************************************************************/
BOOL DrInit(
    LPDIALOGRESIZE lpDr,
    HWND hwndDialog,
    UINT cControls
)
{
    RECT rc;
    
    /* Get the dialog information */
    if(NULL != hwndDialog)
    {
        if(!GetClientRect(hwndDialog, &rc))
            return FALSE;
        
        /* Calculate the original dialog dimensions */
        lpDr->lOrigWidth = rc.right - rc.left;
        lpDr->lOrigHeight = rc.bottom - rc.top;
    }
    lpDr->hwndDialog = hwndDialog;
    
    if(cControls > 0)
    {
        /* Allocate the array of controls */
        lpDr->lpControls = (LPDRCONTROL)HeapAlloc(g_hHeap, 0,
            cControls * sizeof(DRCONTROL));
        if(NULL != lpDr->lpControls)
        {
            UINT iC;
            
            /* Store the control count */
            lpDr->cControls = cControls;
            
            /* Initialize the control array */
            for(iC = 0; iC < cControls; iC ++)
                lpDr->lpControls[iC].hWnd = NULL;
            
            return TRUE;
        }
        else
        {
            lpDr->cControls = 0;
            return FALSE;
        }
    }
    else
    {
        /* Just initialize */
        lpDr->lpControls = NULL;
        lpDr->cControls = 0;
        return TRUE;
    }
}

/******************************************************************************/
VOID DrDestroy(
    LPDIALOGRESIZE lpDr
)
{
    /* Free the array of controls */
    if(NULL != lpDr->lpControls)
    {
        HeapFree(g_hHeap, 0, lpDr->lpControls);
        lpDr->lpControls = NULL;
        lpDr->cControls = 0;
    }
}

/******************************************************************************/
BOOL DrConfigureControl(
    LPDIALOGRESIZE lpDr,
    UINT nControlIdx,
    INT nIDDlgItem,
    WORD wAnchors
)
{
    RECT rc;
    
    if(nControlIdx >= lpDr->cControls)
        return FALSE;
    
    /* Get the control handle */
    lpDr->lpControls[nControlIdx].hWnd = GetDlgItem(lpDr->hwndDialog,
        nIDDlgItem);
    /* Check if control exists */
    if(NULL == lpDr->lpControls[nControlIdx].hWnd)
        return FALSE;
    
    /* Get control dimensions */
    if(!GetWindowRect(lpDr->lpControls[nControlIdx].hWnd, &rc))
    {
        lpDr->lpControls[nControlIdx].hWnd = NULL;
        return FALSE;
    }
    lpDr->lpControls[nControlIdx].lOrigWidth = rc.right - rc.left;
    lpDr->lpControls[nControlIdx].lOrigHeight = rc.bottom - rc.top;
    lpDr->lpControls[nControlIdx].ptOrigPos.x = rc.left;
    lpDr->lpControls[nControlIdx].ptOrigPos.y = rc.top;
    if(!ScreenToClient(lpDr->hwndDialog,
            &(lpDr->lpControls[nControlIdx].ptOrigPos)))
    {
        lpDr->lpControls[nControlIdx].hWnd = NULL;
        return FALSE;
    }
    
    /* Set the anchors */
    lpDr->lpControls[nControlIdx].wAnchors = wAnchors;
    
    return TRUE;
}

/******************************************************************************/
VOID DrDoResize(
    const LPDIALOGRESIZE lpDr
)
{
    UINT iC;
    RECT rcNewSize;
    LONG ldWidth, ldHeight;
    
    if(!GetClientRect(lpDr->hwndDialog, &rcNewSize))
        return;
    
    /* Calculate the dimension differences */
    ldWidth = (rcNewSize.right - rcNewSize.left) - lpDr->lOrigWidth;
    ldHeight = (rcNewSize.bottom - rcNewSize.top) - lpDr->lOrigHeight;
    
    for(iC = 0; iC < lpDr->cControls; iC ++)
    {
        POINT ptPos;
        LONG lWidth, lHeight;
        WORD wAnchors;
        
        /* Check if window was configured */
        if(NULL == lpDr->lpControls[iC].hWnd)
            continue;
        
        /* Get the position and dimensions */
        ptPos = lpDr->lpControls[iC].ptOrigPos;
        lWidth = lpDr->lpControls[iC].lOrigWidth;
        lHeight = lpDr->lpControls[iC].lOrigHeight;
        
        /* Get the anchors */
        wAnchors = lpDr->lpControls[iC].wAnchors;
        
        /* Calculate new horizontal dimensions */
        if((DR_ANCHOR_LEFT | DR_ANCHOR_RIGHT) ==
                (wAnchors & (DR_ANCHOR_LEFT | DR_ANCHOR_RIGHT)))
            lWidth += ldWidth;
        else if(wAnchors & DR_ANCHOR_RIGHT)
            ptPos.x += ldWidth;
        
        /* Calculate new vertical dimensions */
        if((DR_ANCHOR_TOP | DR_ANCHOR_BOTTOM) ==
                (wAnchors & (DR_ANCHOR_TOP | DR_ANCHOR_BOTTOM)))
            lHeight += ldHeight;
        else if(wAnchors & DR_ANCHOR_BOTTOM)
            ptPos.y += ldHeight;
        
        /* Move the control */
        MoveWindow(lpDr->lpControls[iC].hWnd, ptPos.x, ptPos.y,
            lWidth, lHeight, FALSE);
    }

    /* Invalidate the whole dialog */
    InvalidateRect(lpDr->hwndDialog, NULL, TRUE);
}