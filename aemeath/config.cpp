#include "Config.h"
#include <nlohmann/json.hpp>
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include "logger.h"
using json = nlohmann::json;
/// <summary>
/// 获取配置文件的完整路径。函数从当前用户的 AppData（使用 CSIDL_APPDATA）获取目录，并在其后追加文件名 aemeath_config.json。
/// </summary>
/// <returns>返回一个 std::wstring，包含用户的 AppData 目录和追加的文件名 aemeath_config.json（由 SHGetFolderPathW 获取的路径）。</returns>
std::wstring Config::GetConfigPath()
{
    wchar_t appdata[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appdata);
    std::wstring path = appdata;
    path += L"\\aemeath_config.json";
    return path;
}

AppConfig Config::Load()
{
    AppConfig cfg;
    std::wstring path = GetConfigPath();
    std::ifstream in(path);
    if (!in.is_open()) return cfg;

    json j;
    in >> j;
    cfg.windowX = j.value("window_x", cfg.windowX);
    cfg.windowY = j.value("window_y", cfg.windowY);
    cfg.scaleIndex = j.value("scale_index", cfg.scaleIndex);
    cfg.transparencyIndex = j.value("transparency_index", cfg.transparencyIndex);
    cfg.autoStartup = j.value("auto_startup", cfg.autoStartup);
    cfg.clickThrough = j.value("click_through", cfg.clickThrough);
    cfg.followMouse = j.value("follow_mouse", cfg.followMouse);
    cfg.defaultState = j.value("defaultState", cfg.defaultState);
    if (cfg.scaleIndex < 0 || cfg.scaleIndex > 8)
        cfg.scaleIndex = 3;
    if (cfg.transparencyIndex < 0 || cfg.transparencyIndex > 7)
        cfg.transparencyIndex = 0;
#ifdef _DEBUG
        std::stringstream ss;
        ss << "\n[Config::Load] Loaded config:\n"
            << "  window_x          = " << cfg.windowX << "\n"
            << "  window_y          = " << cfg.windowY << "\n"
            << "  scale_index        = " << cfg.scaleIndex << "\n"
            << "  transparency_index = " << cfg.transparencyIndex << "\n"
            << "  auto_startup       = " << (cfg.autoStartup ? "true" : "false") << "\n"
            << "  click_through      = " << (cfg.clickThrough ? "true" : "false") << "\n"
            << "  follow_mouse       = " << (cfg.followMouse ? "true" : "false") << "\n"
            << "  defaultState       = " << (cfg.defaultState ? "true" : "false");
        LOG_INFO(ss.str().c_str());
#endif  
    return cfg;
}

void Config::Save(const AppConfig& cfg)
{
    json j;
    j["scale_index"] = cfg.scaleIndex;
    j["transparency_index"] = cfg.transparencyIndex;
    j["auto_startup"] = cfg.autoStartup;
    j["click_through"] = cfg.clickThrough;
    j["follow_mouse"] = cfg.followMouse;
    j["defaultState"] = cfg.defaultState;
    j["window_x"] = cfg.windowX;
    j["window_y"] = cfg.windowY;
    std::wstring path = GetConfigPath();
#ifdef _DEBUG
        std::stringstream ss;
        ss << "\n[Config::Save] Saving config:\n"
            << "  window_x          = " << cfg.windowX << "\n"
            << "  window_y          = " << cfg.windowY << "\n"
            << "  scale_index        = " << cfg.scaleIndex << "\n"
            << "  transparency_index = " << cfg.transparencyIndex << "\n"
            << "  auto_startup       = " << (cfg.autoStartup ? "true" : "false") << "\n"
            << "  click_through      = " << (cfg.clickThrough ? "true" : "false") << "\n"
            << "  follow_mouse       = " << (cfg.followMouse ? "true" : "false") << "\n"
            << "  defaultState       = " << (cfg.defaultState ? "true" : "false");
        LOG_INFO(ss.str().c_str());
#endif 
    std::ofstream out(path);
    out << j.dump(2);
}
