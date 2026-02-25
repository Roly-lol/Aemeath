#include "TrayIcon.h"
#include "resource.h"
#include "Config.h"


extern AppConfig g_config; 
extern int g_scaleIndex;
extern int g_transparencyIndex;

void TrayIcon::Init(HINSTANCE hInst, HWND Hwnd_)
{
    Hwnd = Hwnd_;

    nid.cbSize = sizeof(nid);
    nid.hWnd = Hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPICON));
    lstrcpyW(nid.szTip, L"飞行雪绒"); 



    Shell_NotifyIcon(NIM_ADD, &nid);
}

void TrayIcon::ShowMenu()
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();

    // 子菜单：缩放
    HMENU scaleMenu = CreatePopupMenu();
    const wchar_t* scaleText[] = { L"30%",L"50%",L"70%",L"90%",L"110%",L"130%",L"150%",L"170%",L"190%" };
    for (int i = 0; i < 9; i++)
        AppendMenuW(scaleMenu,
            (i == g_scaleIndex ? MF_CHECKED : MF_UNCHECKED),
            2000 + i,
            scaleText[i]);

    // 子菜单：透明度
    HMENU alphaMenu = CreatePopupMenu();
    const wchar_t* alphaText[] = { L"100%",L"90%",L"80%",L"70%",L"60%",L"50%",L"40%",L"30%" };
    for (int i = 0; i < 8; i++)
        AppendMenuW(alphaMenu,
            (i == g_transparencyIndex ? MF_CHECKED : MF_UNCHECKED),
            2100 + i,
            alphaText[i]);

    AppendMenuW(menu, MF_POPUP, (UINT_PTR)scaleMenu, L"缩放");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)alphaMenu, L"透明度");

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

    AppendMenuW(menu,
        g_config.followMouse ? MF_CHECKED : MF_UNCHECKED,
        2200, L"跟随鼠标");

    AppendMenuW(menu,
        g_config.clickThrough ? MF_CHECKED : MF_UNCHECKED,
        2201, L"鼠标穿透");
    AppendMenuW(menu,
        g_config.defaultState ? MF_CHECKED : MF_UNCHECKED,
        2202, L"默认静止");

    AppendMenuW(menu,
        g_config.autoStartup ? MF_CHECKED : MF_UNCHECKED,
        2203, L"开机自启");

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

    AppendMenuW(menu, MF_STRING, 2300, L"静止/飞行");
    AppendMenuW(menu, MF_STRING, 2301, L"退出");

    SetForegroundWindow(Hwnd);

    int cmd = TrackPopupMenu(
        menu,
        TPM_RETURNCMD | TPM_NONOTIFY,
        pt.x, pt.y,
        0, Hwnd, nullptr);

    DestroyMenu(menu);

    if (cmd != 0)
        PostMessage(Hwnd, WM_COMMAND, cmd, 0);
}

