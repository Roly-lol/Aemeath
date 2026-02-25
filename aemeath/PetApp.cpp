#include "PetApp.h"
#include "PetWindow.h"

typedef HRESULT(WINAPI* SetProcessDpiAwarenessFunc)(int);

PetApp::PetApp(HINSTANCE hInst) : hInst(hInst) {}

void PetApp::InitDPI()
{
    HMODULE shcore = LoadLibraryW(L"Shcore.dll");
    if (shcore)
    {
        auto fn = (SetProcessDpiAwarenessFunc)GetProcAddress(shcore, "SetProcessDpiAwareness");
        if (fn) fn(2); // PROCESS_PER_MONITOR_DPI_AWARE
        FreeLibrary(shcore);
    }
    else
    {
        SetProcessDPIAware();
    }
}

int PetApp::Run()
{
    InitDPI();

    PetWindow pet(hInst);
    pet.Show();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
