#include "GifPlayer.h"
#include "Logger.h"
#include <string>
GifPlayer::GifPlayer() {}

GifPlayer::~GifPlayer()
{
    for (auto* f : frames) delete f;
    frames.clear();
    if (flipped) delete flipped;
    flipped = nullptr;
}
/// <summary>
/// 对 frames 容器中的每个帧指针调用 delete，释放内存并清空容器。
/// </summary>
void GifPlayer::ClearFrames()
{
    for (auto* f : frames) delete f;
    frames.clear();
}
/// <summary>
/// 加载并解码指定路径的 GIF 文件，将每一帧按给定缩放因子绘制为位图并存入对象的 frames 列表。函数在开始时会清除已有帧，并设置 width 和 height 成员。
/// </summary>
/// <param name="path">指向要加载的 GIF 文件的路径（std::wstring）。函数使用 GDI+ 打开并读取该文件。</param>
/// <param name="scale">缩放因子，用于按比例调整每帧的目标宽度和高度（应大于 0）。</param>
/// <returns>如果成功打开文件、获取帧并至少解码一帧则返回 true；在无法打开文件、获取帧信息、帧计数为 0、或计算出的目标尺寸无效时返回 false。</returns>
bool GifPlayer::Load(const std::wstring& path, double scale)
{
    ClearFrames();

    Gdiplus::Bitmap gif(path.c_str());
    if (gif.GetLastStatus() != Gdiplus::Ok) return false;

    UINT count = gif.GetFrameDimensionsCount();
    if (count == 0) return false;

    GUID* pDimensionIDs = new GUID[count];
    gif.GetFrameDimensionsList(pDimensionIDs, count);
    UINT frameCount = gif.GetFrameCount(&pDimensionIDs[0]);
    if (frameCount == 0)
    {
        delete[] pDimensionIDs;
        return false;
    }

    width = (int)(gif.GetWidth() * scale);
    height = (int)(gif.GetHeight() * scale);
    if (width <= 0 || height <= 0)
    {
        delete[] pDimensionIDs;
        return false;
    }

    for (UINT i = 0; i < frameCount; i++)
    {
        gif.SelectActiveFrame(&pDimensionIDs[0], i);
        Gdiplus::Bitmap* frame = new Gdiplus::Bitmap(width, height);
        Gdiplus::Graphics g(frame);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.DrawImage(&gif, 0, 0, width, height);
        frames.push_back(frame);
    }

    delete[] pDimensionIDs;
    return true;
}
/// <summary>
/// 从指定模块的资源中加载 GIF，按给定缩放比例解码每一帧并存入对象的 frames 列表，同时设置 width 和 height；加载或解码失败时返回 false。
/// </summary>
/// <param name="hInst">包含 GIF 资源的模块实例句柄 (HINSTANCE)。</param>
/// <param name="resID">资源 ID（整数），要加载的 GIF 资源的标识，资源类型为 "GIF"。</param>
/// <param name="scale">缩放因子 (double)，用于按比例调整解码后帧的宽度和高度。</param>
/// <returns>若成功加载并解码 GIF 并生成帧则返回 true；若找不到资源、分配/解码失败、或尺寸无效等情况则返回 false。</returns>
bool GifPlayer::LoadFromResource(HINSTANCE hInst, int resID, double scale)
{
    ClearFrames();

    HRSRC hRes = FindResourceW(hInst, MAKEINTRESOURCEW(resID), L"GIF");
    if (!hRes) return false;

    DWORD size = SizeofResource(hInst, hRes);
    HGLOBAL hMem = LoadResource(hInst, hRes);
    if (!hMem) return false;

    void* pData = LockResource(hMem);
    if (!pData || size == 0) return false;

    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hBuffer) return false;

    void* pBuffer = GlobalLock(hBuffer);
    memcpy(pBuffer, pData, size);
    GlobalUnlock(hBuffer);

    IStream* pStream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(hBuffer, TRUE, &pStream)))
        return false;

    Gdiplus::Bitmap gif(pStream);
    if (gif.GetLastStatus() != Gdiplus::Ok)
    {
        pStream->Release();
        return false;
    }

    UINT count = gif.GetFrameDimensionsCount();
    if (count == 0)
    {
        pStream->Release();
        return false;
    }

    GUID* pDimensionIDs = new GUID[count];
    gif.GetFrameDimensionsList(pDimensionIDs, count);
    UINT frameCount = gif.GetFrameCount(&pDimensionIDs[0]);
    if (frameCount == 0)
    {
        delete[] pDimensionIDs;
        pStream->Release();
        return false;
    }

    width = (int)(gif.GetWidth() * scale);
    height = (int)(gif.GetHeight() * scale);
    if (width <= 0 || height <= 0)
    {
        delete[] pDimensionIDs;
        pStream->Release();
        return false;
    }

    for (UINT i = 0; i < frameCount; i++)
    {
        gif.SelectActiveFrame(&pDimensionIDs[0], i);
        Gdiplus::Bitmap* frame = new Gdiplus::Bitmap(width, height);
        Gdiplus::Graphics g(frame);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.DrawImage(&gif, 0, 0, width, height);
        frames.push_back(frame);
    }

    delete[] pDimensionIDs;
    pStream->Release();


    // 检查第一帧的透明度
    Gdiplus::Bitmap* bmp = frames[0];
    Gdiplus::BitmapData data;
    Gdiplus::Rect rect(0, 0, width, height);

    bmp->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);

    BYTE* p = (BYTE*)data.Scan0;
    int transparentCount = 0;
    int total = width * height;

    for (int i = 0; i < total; i++)
    {
        BYTE A = p[i * 4 + 3];
        if (A == 0) transparentCount++;
    }

    bmp->UnlockBits(&data);
#ifdef _DEBUG
    char msg[128];
    sprintf_s(msg, "GIF_ID[%d] 加载成功，帧数 = %u 宽 = %d 高 = %d 透明像素 = %d / %d", resID,frameCount, width, height, transparentCount, total);
    LOG_DEBUG(msg);
#endif
    return true;
}
void GifPlayer::BuildFlipped()
{
    if (frames.empty())
        return;

    // 如果之前已经建过，先清掉
    if (flipped)
    {
        delete flipped;
        flipped = nullptr;
    }

    flipped = new GifPlayer();
    flipped->width = width;
    flipped->height = height;
    flipped->globalAlpha = globalAlpha;

    for (auto* src : frames)
    {
        if (!src) continue;

        // 创建与当前尺寸一致的新位图
        Gdiplus::Bitmap* dst = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
        Gdiplus::Graphics g(dst);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        // 先画过去，再水平翻转
        g.DrawImage(src, 0, 0, width, height);
        dst->RotateFlip(Gdiplus::RotateNoneFlipX);

        flipped->frames.push_back(dst);
    }
}
/// <summary>
/// 将指定的 GIF 帧按每像素 alpha 和全局透明度绘制到分层窗口上；若没有帧则不执行任何操作。
/// </summary>
/// <param name="Hwnd">目标窗口的 HWND（应为分层窗口）。</param>
/// <param name="frameIndex">要绘制的帧索引；会对 frames.size() 取模以循环访问帧。若 frames 为空，函数直接返回并不绘制。</param>
/// <param name="x">目标绘制位置的 x 坐标（当前实现中未被使用）。</param>
/// <param name="y">目标绘制位置的 y 坐标（当前实现中未被使用）。</param>
void GifPlayer::DrawFrame(HWND Hwnd, int frameIndex,int x,int y)
{
    if (frames.empty()) return;
    frameIndex %= frames.size();

    Gdiplus::Bitmap* bmp = frames[frameIndex];

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hBmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);

    Gdiplus::BitmapData data;
    Gdiplus::Rect rect(0, 0, width, height);
    bmp->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);

    BYTE* src = (BYTE*)data.Scan0;
    BYTE* dst = (BYTE*)bits;

    for (int i = 0; i < width * height; i++)
    {
        BYTE B = src[0];
        BYTE G = src[1];
        BYTE R = src[2];
        BYTE A = src[3];

        // ⭐ 全局透明度融合
        BYTE finalA = (BYTE)(A * globalAlpha / 255);

        dst[0] = (BYTE)(B * finalA / 255);
        dst[1] = (BYTE)(G * finalA / 255);
        dst[2] = (BYTE)(R * finalA / 255);
        dst[3] = finalA;

        src += 4;
        dst += 4;
    }

    bmp->UnlockBits(&data);

    HDC hdc = GetDC(Hwnd);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBmp);

    SIZE size = { width, height };
    POINT ptSrc = { 0, 0 };

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	// 使用 UpdateLayeredWindow 绘制到分层窗口，支持每像素 alpha 和全局透明度
    UpdateLayeredWindow(Hwnd, hdc, nullptr, &size, memDC, &ptSrc, 0, &bf, ULW_ALPHA);

    SelectObject(memDC, oldBmp);
    DeleteObject(hBmp);
    DeleteDC(memDC);
    ReleaseDC(Hwnd, hdc);
}


