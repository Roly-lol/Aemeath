#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>

class GifLoader
{
public:
    bool Load(const wchar_t* path);
    void DrawFrame(HWND hWnd, int index, int x, int y);
    int GetFrameCount() const { return frames.size(); }
    int Width() const { return width; }
    int Height() const { return height; }

private:
    std::vector<Gdiplus::Bitmap*> frames;
    int width = 0, height = 0;
};
