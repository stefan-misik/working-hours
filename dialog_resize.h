#ifndef DIALOG_RESIZE_H
#define DIALOG_RESIZE_H

#include "win_common.h"
 
#define DR_ANCHOR_LEFT     0x1
#define DR_ANCHOR_TOP      0x2
#define DR_ANCHOR_RIGHT    0x4
#define DR_ANCHOR_BOTTOM   0x8


/**
 * @brief Structure holding information on individual controls
 * 
 */
typedef struct tagDRCONTROL
{
    WORD wAnchors;      /**< Bitfield describing the control's anchors */     
    POINT ptOrigPos;    /**< Control's original position */
    LONG lOrigWidth;    /**< Control's original width */
    LONG lOrigHeight;   /**< Control's original height */
    HWND hWnd;          /**< Control's handle */
}DRCONTROL, *LPDRCONTROL;

/**
 * @brief Information about resizing dialog window
 * 
 */
typedef struct tagDIALOGRESIZE
{
    LONG lOrigWidth;        /**< Dialog's original width */
    LONG lOrigHeight;       /**< Dialog's original height */
    HWND hwndDialog;        /**< Main dialog handle */
    LPDRCONTROL lpControls; /**< Array of controls to be resized */
    UINT cControls;         /**< Count of controls in the array */
} DIALOGRESIZE, *LPDIALOGRESIZE;


/**
 * @brief Initialize dialog resize object
 * 
 * @param[out] lpDr Dialog resize information structure to be initialized
 * @param hwndDialog Dialog window handle; can be NULL to just initialize the
 *                   structure
 * @param cControls Maximum number of controls resized by this object; can be 0
 *                  to just initialize the structure
 * 
 * @return FALSE on failure
 */
BOOL DrInit(
    LPDIALOGRESIZE lpDr,
    HWND hwndDialog,
    UINT cControls
);

/**
 * @brief Destroy dialog resize object
 * 
 * @param[in] lpDr Dialog resize information object
 */
VOID DrDestroy(
    LPDIALOGRESIZE lpDr
);

/**
 * @brief Configure single control
 * 
 * @param[in,out] lpDr Dialog resize object
 * @param nControlIdx Control index inside dialog resize object
 * @param nIDDlgItem Dialog item item ID
 * @param wAnchors Combination of anchors to be applied on the control
 * 
 * @return FALSE on failure
 */
BOOL DrConfigureControl(
    LPDIALOGRESIZE lpDr,
    UINT nControlIdx,
    INT nIDDlgItem,
    WORD wAnchors
);

/**
 * @brief Move / resize controls in the window in response to WM_SIZE
 * 
 * @param[in] lpDr Dialog resize object
 */
VOID DrDoResize(
    const LPDIALOGRESIZE lpDr
);

#endif /* DIALOG_RESIZE_H */

