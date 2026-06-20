#include "PetWindow.h"
#include "Config.h"
#include "TrayIcon.h"
#include <windowsx.h>
#include "resource.h"
#pragma comment(lib, "gdiplus.lib")
// 全局状态（给托盘菜单用）
AppConfig g_config;
int g_scaleIndex = 3;
int g_transparencyIndex = 0;
// 初始化 GDI+、加载配置、注册窗口类、创建窗口和加载 GIF
PetWindow::PetWindow(HINSTANCE hInst) : hInst(hInst), tray()
{
    //设置原子锁
    PetWindow::CheckSingleInstance();
    // GDI+
    Gdiplus::GdiplusStartupInput gsi;
    GdiplusStartup(&gdiplusToken, &gsi, nullptr);

    //配置加载
    cfg = Config::Load();
    g_config = cfg;
    g_scaleIndex = cfg.scaleIndex;
    g_transparencyIndex = cfg.transparencyIndex;

    //防止配置里存了越界值
    if (cfg.scaleIndex < 0 || cfg.scaleIndex > 8)
        cfg.scaleIndex = g_scaleIndex = 3;   // 默认中间值
    if (cfg.transparencyIndex < 0 || cfg.transparencyIndex > 7)
        cfg.transparencyIndex = g_transparencyIndex = 0; // 默认不透明

    //窗口注册
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"aemeath";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);
    //窗口创建
    Hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,wc.lpszClassName, L"",WS_POPUP, cfg.windowX, cfg.windowY, 200, 200, nullptr, nullptr, hInst, this);
    // 加载 GIF（从资源）
    LoadAllGifs();

    //绘制
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    currentGif = &moveRight;
    motion = new MotionSystem(sw, sh, moveRight.Width(), moveRight.Height());
	motion->SetPosition(cfg.windowX, cfg.windowY);//从配置里读取上次关闭时的位置
    
    ApplyClickThrough();
	// 首次绘制，确保窗口大小正确（有时第一次加载 GIF 时窗口大小可能不对）
    currentGif->DrawFrame(Hwnd, 0, 0, 0);
	// 应用配置的透明度设置
    SetTransparency(cfg.transparencyIndex);

    tray.Init(hInst, Hwnd);
    if (cfg.defaultState) TogglePause();
}
// 加载所有 GIF，根据当前缩放设置调整大小
void PetWindow::LoadAllGifs()
{
    double scaleOptions[] = { 0.3,0.5,0.7,0.9,1.1,1.3,1.5,1.7,1.9 };
    double scale = scaleOptions[g_scaleIndex];

    LoadGif(moveRight, IDR_GIF_MOVE, scale);
    moveRight.BuildFlipped();   // 生成反转版本
    currentGif = &moveRight;    // 默认朝右

    LoadGif(idle[0], IDR_GIF_IDLE1, scale);
    LoadGif(idle[1], IDR_GIF_IDLE2, scale);
    LoadGif(idle[2], IDR_GIF_IDLE3, scale);
    LoadGif(idle[3], IDR_GIF_IDLE4, scale);
    LoadGif(dragGif, IDR_GIF_DRAG, scale);
}
// 从资源加载 GIF
void PetWindow::LoadGif(GifPlayer& gif, int id, double scale)
{
    gif.LoadFromResource(hInst, id, scale);
}
// 显式释放所有 GIF 帧，避免析构时 GDI+ 已销毁
void PetWindow::DestroyAllGifs()
{
    moveRight.ClearFrames();
    moveLeft.ClearFrames();
    dragGif.ClearFrames();
    for (auto& g : idle) g.ClearFrames();
}
// 显示窗口并启动定时器
void PetWindow::Show()
{
    ShowWindow(Hwnd, SW_SHOW);
    UpdateWindow(Hwnd);

    SetTimer(Hwnd, 1, 40, nullptr);  // 动画
    SetTimer(Hwnd, 2, 30, nullptr);  // 运动
}

// 静态窗口过程函数，转发消息到HandleMessage
LRESULT CALLBACK PetWindow::WndProc(HWND Hwnd, UINT msg, WPARAM w, LPARAM l)
{
    PetWindow* self = nullptr;
    if (msg == WM_NCCREATE)
    {
        auto cs = (CREATESTRUCT*)l;
        self = (PetWindow*)cs->lpCreateParams;
        SetWindowLongPtr(Hwnd, GWLP_USERDATA, (LONG_PTR)self);
        self->Hwnd = Hwnd;
    }
    else
    {
        self = (PetWindow*)GetWindowLongPtr(Hwnd, GWLP_USERDATA);
    }
    if (!self) return DefWindowProc(Hwnd, msg, w, l);
    return self->HandleMessage(msg, w, l);
}
// 处理窗口消息，根据消息类型执行对应操作
LRESULT PetWindow::HandleMessage(UINT msg, WPARAM w, LPARAM l)
{
    switch (msg)
    {
    case WM_CREATE:
       {
           HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPICON));
           SendMessage(Hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
           SendMessage(Hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
           return 0;
       }
    case WM_TRAYICON:
        if (l == WM_RBUTTONUP)
            tray.ShowMenu();
        return 0;

    case WM_TIMER:
        OnTimer((UINT)w);   
        return 0;

    case WM_LBUTTONDOWN:
        if (!cfg.clickThrough)
        {
            POINT pt{ GET_X_LPARAM(l), GET_Y_LPARAM(l) };
            StartDrag(pt);
        }
        return 0;

    case WM_MOUSEMOVE:
        if (dragging)
        {
            POINT pt{ GET_X_LPARAM(l), GET_Y_LPARAM(l) };
            //DoDrag(pt);
        }
        return 0;

    case WM_LBUTTONUP:
        StopDrag();
        return 0;

    case WM_COMMAND:
        HandleCommand((int)w);
        return 0;

    case WM_DESTROY:
        SaveLocation();
        KillTimer(Hwnd, 1);
        KillTimer(Hwnd, 2);
        DestroyAllGifs();
        Gdiplus::GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(Hwnd, msg, w, l);
}
// 处理托盘菜单命令，根据 ID 执行对应操作
void PetWindow::HandleCommand(int id)
{
    // 缩放
    if (id >= 2000 && id <= 2008) { SetScale(id - 2000); return; }

    // 透明度
    if (id >= 2100 && id <= 2107) { SetTransparency(id - 2100); return; }

    // 跟随
    if (id == 2200)
    {
        cfg.followMouse = !cfg.followMouse;
        g_config = cfg;
        Config::Save(cfg);
        return;
    }

    // 穿透
    if (id == 2201)
    {
        cfg.clickThrough = !cfg.clickThrough;
        g_config = cfg;
        Config::Save(cfg);
        ApplyClickThrough();
        return;
    }
    //默认
    if (id == 2202)
    {
        cfg.defaultState = !cfg.defaultState;
        g_config = cfg;
        Config::Save(cfg);
        return;
    }
    // 自启
    if (id == 2203)
    {
        cfg.autoStartup = !cfg.autoStartup;
        g_config = cfg;
        Config::Save(cfg);
        SetAutoStartup(cfg.autoStartup);
        return;
    }

    // 暂停
    if (id == 2300)
    {
        TogglePause();
        return;
    }

    // 退出
    if (id == 2301)
    {
        DestroyWindow(Hwnd);
        return;
    }
}
// 处理定时器消息，分别更新动画帧和运动系统
void PetWindow::OnTimer(UINT id)
{   // 动画帧更新
    if (id == 1)
    {
        UpdateFrame();
        return;
    }
	// 运动更新和拖动处理
    if (id == 2)
    {
		// 如果正在拖动但鼠标左键已松开，结束拖动
        if (dragging && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
        {
            dragging = false;
            ReleaseCapture();
            StopDrag(); 
            return;
        }
		// 如果正在拖动，更新窗口位置和运动系统位置
        if (dragging)
        {
            POINT pt;
            GetCursorPos(&pt);

            prevMousePos = lastMousePos;
            lastMousePos = pt;

            int newX = pt.x - dragOffset.x;
            int newY = pt.y - dragOffset.y;
            RECT rc;
            GetWindowRect(Hwnd, &rc);
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;
            MoveWindow(Hwnd, newX, newY, w, h, FALSE);
            motion->SetPosition(newX, newY);

            return;
        }
		// 如果没有拖动，正常更新运动系统
        UpdateMotion();
        return;
    }
}
// 更新动画帧索引并重绘当前帧
void PetWindow::UpdateFrame() 
{ 
    if (!currentGif) return; 
    size_t count = currentGif->GetFrameCount(); 
    if (count == 0) return; 

    frameIndex = (frameIndex + 1) % count; 
    currentGif->DrawFrame(Hwnd, frameIndex,0,0);
}
// 更新运动系统并根据运动方向切换动画
void PetWindow::UpdateMotion()
{
    if (!motion)return;
    if (isPaused || dragging)return;

    POINT mouse;
    GetCursorPos(&mouse);
    motion->Update(cfg.followMouse, mouse);

    double x, y;
    motion->GetPosition(x, y);

    SetWindowPos(Hwnd,HWND_TOPMOST, static_cast<int>(x), static_cast<int>(y),0,0,SWP_NOSIZE | SWP_NOACTIVATE);
    if (motion->IsMovingRight())
        currentGif = &moveRight;
    else
        currentGif = moveRight.GetFlipped();
}

// 根据配置应用或取消点击穿透效果
void PetWindow::ApplyClickThrough()
{
    LONG ex = GetWindowLongW(Hwnd, GWL_EXSTYLE);
    ex |= WS_EX_LAYERED;

    if (cfg.clickThrough)
        ex |= WS_EX_TRANSPARENT;
    else
        ex &= ~WS_EX_TRANSPARENT;

    SetWindowLongW(Hwnd, GWL_EXSTYLE, ex);
}
// 开始拖动，记录偏移和切换到拖动动画
void PetWindow::StartDrag(POINT pt)
{
    dragging = true;
    POINT screenPt = pt;
    ClientToScreen(Hwnd, &screenPt);

    double x, y;
    motion->GetPosition(x, y);
    dragOffset.x = (LONG)(screenPt.x - x);
    dragOffset.y = (LONG)(screenPt.y - y);

    currentGif = &dragGif;
    frameIndex = 0;
}
// 拖动过程中更新窗口位置和运动系统位置
void PetWindow::DoDrag(POINT pt)
{
    POINT screenPt = pt;
    ClientToScreen(Hwnd, &screenPt);
    double nx = screenPt.x - dragOffset.x;
    double ny = screenPt.y - dragOffset.y;
    motion->SetPosition(nx, ny);
    SetWindowPos(Hwnd, HWND_TOPMOST, (int)nx, (int)ny, 0, 0,
        SWP_NOSIZE | SWP_NOACTIVATE);
}
// 结束拖动，计算惯性并应用
void PetWindow::StopDrag()
{
    dragging = false;
    ReleaseCapture();

    double vx = (lastMousePos.x - prevMousePos.x);
    double vy = (lastMousePos.y - prevMousePos.y);

    double inertia = 0.6;
    vx *= inertia;
    vy *= inertia;

    motion->ApplyExternalVelocity(vx, vy);

    if (isPaused)
	{   // 如果暂停，随机选择一个闲置动画
        currentGif = &idle[rand() % 4];
        frameIndex = 0;
        return;
    }
    if (motion->IsMovingRight())
        currentGif = &moveRight;
    else
        currentGif = moveRight.GetFlipped();
    frameIndex = 0;
}
// 根据选择的缩放索引更新所有 GIF 的缩放
void PetWindow::SetScale(int index)
{
    g_scaleIndex = index;
    cfg.scaleIndex = index;
    g_config = cfg;
    Config::Save(cfg);

    LoadAllGifs();
    currentGif->DrawFrame(Hwnd, 0, 0, 0);
}
// 根据选择的透明度索引更新窗口和 GIF 的透明度
void PetWindow::SetTransparency(int index)
{
    g_transparencyIndex = index;
    cfg.transparencyIndex = index;
    g_config = cfg;
    Config::Save(cfg);

    double alphaOptions[] = { 1.0,0.9,0.8,0.7,0.6,0.5,0.4,0.3 };
    BYTE alpha = static_cast<BYTE>(alphaOptions[index] * 255);

    moveRight.SetGlobalAlpha(alpha);
    moveLeft.SetGlobalAlpha(alpha);
    dragGif.SetGlobalAlpha(alpha);
    for (auto& gif : idle)
        gif.SetGlobalAlpha(alpha);

    currentGif->DrawFrame(Hwnd, frameIndex, 0, 0);
}
// 暂停或恢复动画和运动
void PetWindow::TogglePause()
{
    isPaused = !isPaused;

    if (isPaused)
	{// 暂停时随机选择一个闲置动画
        currentGif = &idle[rand() % 4];
        frameIndex = 0;
    }
    else
    {
        if (motion->IsMovingRight())
            currentGif = &moveRight;
        else
            currentGif = moveRight.GetFlipped();

        frameIndex = 0;
    }
}
// 设置或取消开机自启
void PetWindow::SetAutoStartup(bool enable)
{
    HKEY key;
    RegOpenKeyW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        &key);

    if (enable)
    {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        RegSetValueExW(key, L"DesktopPet", 0, REG_SZ,
            (BYTE*)exePath,
            (DWORD)((wcslen(exePath) + 1) * sizeof(wchar_t)));
    }
    else
    {
        RegDeleteValueW(key, L"DesktopPet");
    }

    RegCloseKey(key);
}
//保存当前窗口位置到配置文件，以便下次启动时恢复位置
void PetWindow::SaveLocation()
{
    // 1. 获取窗口位置
    RECT rc;
    GetWindowRect(Hwnd, &rc);
    cfg.windowX = rc.left;
    cfg.windowY = rc.top;
    Config::Save(cfg);
}
//设置原子锁
void PetWindow::CheckSingleInstance()
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, L"aemeath");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(NULL, L"程序已经在运行中！", L"提示", MB_ICONINFORMATION);
        CloseHandle(hMutex);
        exit(0);
    }
}