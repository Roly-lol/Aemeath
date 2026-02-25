#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>

class GifPlayer
{
public:
    GifPlayer();
    ~GifPlayer();
    void ClearFrames();
    bool LoadFromResource(HINSTANCE hInst, int resID, double scale);
    bool Load(const std::wstring& path, double scale = 1.0);
    void DrawFrame(HWND Hwnd, int frameIndex, int x, int y);
    BYTE globalAlpha = 255;   // 默认不透明
    void SetGlobalAlpha(BYTE a) { globalAlpha = a; }
    int GetFrameCount() const { return (int)frames.size(); }
    int Width() const { return width; }
    int Height() const { return height; }
    // 水平翻转版本
    void BuildFlipped();
    GifPlayer* GetFlipped() { return flipped; }


private:
    std::vector<Gdiplus::Bitmap*> frames;
    int width = 0, height = 0;
    GifPlayer* flipped = nullptr;
};
