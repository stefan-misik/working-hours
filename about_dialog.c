#include "about_dialog.h"
#include "defs.h"
#include "resource.h"

/******************************************************************************/
static INT_PTR CALLBACK AboutProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {

    case WM_INITDIALOG:
    {
        HICON hAppIco;

        hAppIco = (HICON)LoadImage(
            g_hInstance,
            MAKEINTRESOURCE(IDI_MAIN),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXICON) * 2,
            GetSystemMetrics(SM_CYICON) * 2,
            0);        

        if(NULL != hAppIco)
        {
            SendDlgItemMessage(hwndDlg, IDC_APP_ICON, STM_SETICON, 
                (WPARAM) hAppIco, 0);                        
        }

        SendDlgItemMessage(hwndDlg, IDC_LICTEXT, WM_SETFONT, 
            (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), (LPARAM)TRUE);

        SetDlgItemText(hwndDlg, IDC_APP_NAME, lpProjectName);
        SetDlgItemText(hwndDlg, IDC_APP_DESC, lpProjectDesc);
        SetDlgItemText(hwndDlg, IDC_APP_VER, lpProjectVer);
        SetDlgItemText(hwndDlg, IDC_APP_GIT, lpProjectGit);
        SetDlgItemText(hwndDlg, IDC_APP_AUTHOR, lpProjectAuthor);             
        SetDlgItemText(hwndDlg, IDC_LICTEXT, lpProjectLic);

        SetFocus(GetDlgItem(hwndDlg, IDOK));

        return TRUE;
    }

    case WM_DESTROY:
    {
        HICON hIcon;
        
        /* Destroy icon */
        hIcon = (HICON)SendDlgItemMessage(hwndDlg, IDC_APP_ICON, STM_GETICON, 
            0, 0);
        if(NULL != hIcon)
        {
            DestroyIcon(hIcon);
        }
        
        return TRUE;
    }

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case NM_CLICK:
        case NM_RETURN:			
            switch (((LPNMHDR)lParam)->idFrom)
            {
            case IDC_GET_LATEST:
                ShellExecute(NULL, TEXT("open"), lpProjectWeb, NULL,
                    NULL, SW_SHOW);
                return TRUE;
            case IDC_MAILTO:			
                ShellExecute(NULL, TEXT("open"),
                    TEXT("mailto:") TEXT(PROJECT_MAIL)
                    TEXT("?subject=") TEXT(PROJECT_NAME), NULL,
                    NULL, SW_SHOW);
                return TRUE;
            }
            break;
        }        
        return FALSE;

    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case 0:
            switch (LOWORD(wParam))
            {
            case IDOK:
            case IDCANCEL:
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
            
            case IDC_GET_LATEST:
                return TRUE;
            }
        }
    }
    }
    return FALSE;    
}

/******************************************************************************/
VOID ShowAboutDialog(
    HWND hwndParent   
)
{
    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), 
        hwndParent, AboutProc);
}
