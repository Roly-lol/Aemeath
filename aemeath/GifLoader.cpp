#include "GifLoader.h"

bool GifLoader::Load(const wchar_t* path)
{
    Gdiplus::Bitmap gif(path);
    if (gif.GetLastStatus() != Gdiplus::Ok) return false;

    width = gif.GetWidth();
    height = gif.GetHeight();

    UINT count = gif.GetFrameDimensionsCount();
    GUID* pDimensionIDs = new GUID[count];
    gif.GetFrameDimensionsList(pDimensionIDs, count);

    UINT frameCount = gif.GetFrameCount(&pDimensionIDs[0]);

    for (UINT i = 0; i < frameCount; i++)
    {
        gif.SelectActiveFrame(&pDimensionIDs[0], i);
        Gdiplus::Bitmap* frame = new Gdiplus::Bitmap(width, height);
        Gdiplus::Graphics g(frame);
        g.DrawImage(&gif, 0, 0);
        frames.push_back(frame);
    }

    delete[] pDimensionIDs;
    return true;
}

void GifLoader::DrawFrame(HWND hWnd, int index, int x, int y)
{
    HDC hdc = GetDC(hWnd);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp;
    frames[index]->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hBmp);

    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBmp);

    SIZE size = { width, height };
    POINT ptSrc = { 0, 0 };
    POINT ptDst = { x, y };

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    UpdateLayeredWindow(
        hWnd, hdc, &ptDst, &size, memDC,
        &ptSrc, 0, &bf, ULW_ALPHA
    );

    SelectObject(memDC, oldBmp);
    DeleteObject(hBmp);
    DeleteDC(memDC);
    ReleaseDC(hWnd, hdc);
}
