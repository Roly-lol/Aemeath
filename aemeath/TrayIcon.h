#pragma once
#include <windows.h>

#define WM_TRAYICON (WM_USER + 1)

class TrayIcon
{
public:
    void Init(HINSTANCE hInst, HWND Hwnd);
    void ShowMenu();

private:
    NOTIFYICONDATA nid{};
    HWND Hwnd = nullptr;
};
