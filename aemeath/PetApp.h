#pragma once
#include <windows.h>

class PetApp
{
public:
    PetApp(HINSTANCE hInst);
    int Run();

private:
    void InitDPI();
    HINSTANCE hInst;
};
