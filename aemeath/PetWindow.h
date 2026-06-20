#include <windows.h>
#include <gdiplus.h>
#include "GifPlayer.h"
#include "MotionSystem.h"
#include "Config.h"
#include "TrayIcon.h"

class PetWindow
{
public:
    PetWindow(HINSTANCE hInst);
    void LoadGif(GifPlayer& gif, int id, double scale);
    void LoadAllGifs();
    void Show();

private:
    // Win32 消息处理
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT HandleMessage(UINT msg, WPARAM w, LPARAM l);
    void HandleCommand(int id);
    // 定时器
    void OnTimer(UINT id);
    void UpdateFrame();
    void UpdateMotion();

    // 窗口行为
    void ApplyClickThrough();
    void StartDrag(POINT pt);
    void DoDrag(POINT pt);
    void StopDrag();
    void SaveLocation();
    // 托盘菜单功能
    void SetScale(int index);
    void SetTransparency(int index);
    void TogglePause();
    void SetAutoStartup(bool enable);
    // 原子锁
    void CheckSingleInstance();
    //释放所有 GDI+ 对象
    void DestroyAllGifs();
private:
    // 基础
    HINSTANCE hInst;
    HWND Hwnd = nullptr;
    ULONG_PTR gdiplusToken;

    // 配置
    AppConfig cfg;

    // GIF 动画
    GifPlayer moveRight, moveLeft;
    GifPlayer idle[4];
    GifPlayer dragGif;
    GifPlayer* currentGif = nullptr;
    int frameIndex = 0;

    // 运动系统
    MotionSystem* motion = nullptr;
    POINT lastMousePos = { 0, 0 };
    POINT prevMousePos = { 0, 0 };

    // 拖动
    bool dragging = false;
    POINT dragOffset{0,0};
    bool isPaused = false;


    // 托盘
    TrayIcon tray;
};
