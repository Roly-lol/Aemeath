#include <windows.h>
#include "PetWindow.h"

#ifdef _DEBUG
#include "Logger.h"
#include <shlobj_core.h>
#endif

void AttachConsole()
{
    AllocConsole() && ::AttachConsole(GetCurrentProcessId());
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    auto consoleWindow = GetConsoleWindow();
    SetForegroundWindow(consoleWindow);
    ShowWindow(consoleWindow, SW_RESTORE);
    ShowWindow(consoleWindow, SW_SHOW);
}
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
#ifdef _DEBUG
    AttachConsole();
    Logger::SetLevel(Logger::Level::Debug, Logger::LoggerType::ConsoleLogger);
    if (IsUserAnAdmin())
    {
        LOG_INFO("具有管理员权限");
    }
    else
    {
        LOG_INFO("不具有管理员权限");
    }
#endif
	// 创建并显示宠物窗口
    PetWindow app(hInstance);
    app.Show();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}