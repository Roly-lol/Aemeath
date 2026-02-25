#pragma once
#include <string>

struct AppConfig
{
    int scaleIndex = 3;
    int transparencyIndex = 0;
    int windowX = 500;
    int windowY = 500;
    bool autoStartup = false;
    bool clickThrough = false;
    bool followMouse = false;
    bool defaultState = true;
};

class Config
{
public:
    static AppConfig Load();
    static void Save(const AppConfig& cfg);

private:
    static std::wstring GetConfigPath();
};
