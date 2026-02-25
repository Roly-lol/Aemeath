# Aemeath (飞行雪绒)

项目概述
- `Aemeath` 是一个基于 Win32 + GDI+ 的桌面宠物/动画演示程序，使用分层窗口（`WS_EX_LAYERED`）实现每像素 alpha 混合。
- GIF 的解码、缩放与逐帧渲染由 `GifPlayer` / `GifLoader` 实现。托盘交互、配置保存、运动逻辑等构成完整功能。

主要依赖
- Windows 10/11
- Visual Studio 2019/2022（MSVC）
- 链接 `gdiplus.lib`（项目已在代码中使用 `#pragma comment(lib, "gdiplus.lib")`）
- `nlohmann/json` 用于配置文件读写（包已包含于 `packages`/`nuget`）

如何构建
1. 在 Visual Studio 中打开解决方案（`ameath.sln`）。
2. 选择目标平台（`x86` 或 `x64`），选择配置（`Debug` / `Release`）。
3. Build → Build Solution。若资源修改后出现问题，请 Clean + Rebuild。

项目结构（重要源文件说明）
- `main.cpp` — 程序入口，调试模式下附加控制台并启动主窗口。
- `PetApp.*` — DPI 与应用启动包装（可选入口变体）。
- `PetWindow.h/.cpp` — 主窗口逻辑：创建分层窗口、加载 GIF、计时器、消息处理与命令分发。
- `GifPlayer.h/.cpp` — 从资源或文件加载 GIF，按缩放解码每帧、构建翻转版本、按像素与全局 alpha 渲染到分层窗口。
- `GifLoader.h/.cpp` — 较简单的 GIF 从文件加载并绘制到指定窗口（备用实现）。
- `MotionSystem.h/.cpp` — 宠物运动与物理（边界、速度、朝向等）。
- `TrayIcon.h/.cpp` — 系统托盘图标与菜单（缩放、透明度、开关项）。
- `Config.h/.cpp` — 配置持久化（保存到 `%APPDATA%\\aemeath_config.json`）。
- `Logger.h/.cpp` — 简单日志工具（在调试时输出到控制台）。
- `Resource.rc`、`resource.h` — 资源定义（图标与 GIF 资源）。

资源和 GIF 存放
- 项目中 GIF 文件位于 `ameath/gifs/`，并在 `Resource.rc` 中以 `GIF` 类型嵌入。例如：
  - `IDR_GIF_MOVE GIF "gifs/move.gif"`
  - `IDR_GIF_IDLE1 GIF "gifs/idle1.gif"`

如何添加并调用新的 GIF 动画（推荐：作为资源）
1. 将 GIF 文件放入 `ameath/gifs/`（例如 `gifs/new_anim.gif`）。
2. 在 `ameath/resource.h` 中新增资源 ID：
   ```cpp
   #define IDR_GIF_NEW 10007
   ```
3. 在 `ameath/Resource.rc` 中添加：
   ```rc
   IDR_GIF_NEW GIF "gifs/new_anim.gif"
   ```
4. 在 `PetWindow.h` 添加成员变量：
   ```cpp
   GifPlayer newGif;
   ```
5. 在 `PetWindow::LoadAllGifs()` 内按现有模式加载：
   ```cpp
   LoadGif(newGif, IDR_GIF_NEW, scale);
   newGif.BuildFlipped(); // 若需要左右反转
   ```
6. 运行时切换显示：
   ```cpp
   currentGif = &newGif; // 绘制循环会开始使用它
   ```

在托盘菜单中添加切换项（示例）
- 在 `TrayIcon::ShowMenu()` 中添加菜单项：
  ```cpp
  AppendMenuW(menu, MF_STRING, 2400, L"切换到 New 动画");
  ```
- 在 `PetWindow::HandleCommand(int id)` 中处理：
  ```cpp
  if (id == 2400) { currentGif = &newGif; return; }
  ```

从文件运行时加载（动态加载，非资源）
- 创建 `GifPlayer` 后调用 `Load`：
  ```cpp
  GifPlayer runtimeGif;
  if (runtimeGif.Load(L"C:\\path\\to\\file.gif", 1.0)) {
      runtimeGif.BuildFlipped();
      currentGif = &runtimeGif;
  }
  ```
- 注意：`runtimeGif` 变量必须在其生命周期内持续存在（不能是临时变量），否则 `currentGif` 会悬空。

缩放与透明度说明
- 缩放选项定义在 `PetWindow::LoadAllGifs()` 的 `scaleOptions` 数组中，`g_scaleIndex` 从 `Config` 中读取并可通过托盘菜单修改。
- 透明度通过 `GifPlayer::globalAlpha` 与 `UpdateLayeredWindow` 的 per-pixel alpha 组合实现；托盘菜单修改会保存到 `Config` 并立刻生效。

调试与常见问题
- 资源未生效：清理（Clean）然后完全重建（Rebuild）。
- GIF 加载失败：确保 `Resource.rc` 的路径正确，或者运行时加载时路径存在且进程有读取权限。
- 运行时 GIF 显示异常：检查 `scale` 参数与 `GifPlayer` 的 `width`/`height` 是否合理。

贡献说明
- 欢迎提交 PR：请保持代码风格一致，修改 UI 或资源后附带简要说明。

许可
- 本项目采用 MIT 许可证

