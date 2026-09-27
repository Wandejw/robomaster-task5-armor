# RoboMaster Task5 装甲板识别

## 当前版本：框架完整

初步完成图像处理（hsv+二值化）与灯条识别。

目前实现：
- 视频逐帧读取与预览。
- BGR 转 HSV，按颜色、饱和度和明度范围生成二值 mask。
- 外轮廓提取、旋转矩形拟合。
- 按长宽比和填充率筛选单灯条候选，并绘制调试框。

尚未完成：灯条配对、装甲板结果数据及最终原图绘制。主程序保留这些模块的调用骨架；当前可使用独立预览程序检查预处理和灯条候选。快速晃动时仍存在闪烁和漏检，需要继续验证和调整。

## 模块

- `main/`：主程序流程。
- `processing/`：HSV 预处理。
- `detection/`：轮廓、拟合与单灯条筛选。
- `matching/`：灯条配对骨架。
- `InputHandle/`：绘制输出骨架。
- `common/`：共用类型预留。
- `preview/`：可运行的调试预览程序。

## 构建与预览

依赖：C++17、CMake、OpenCV（当前环境为 4.6.0）。

```bash
cmake -S preview -B build/preview
cmake --build build/preview --parallel 2
./build/preview/preprocess_preview
```

按 Esc 退出。视频未纳入仓库，运行前请修改 `preview/preview.cpp` 中的 `videoPath` 为本机视频路径。

完整框架构建：

```bash
cmake -S . -B build
cmake --build build --parallel 2
```

完整框架编译通过不代表配对、绘制算法已经实现，暂以预览程序进行运行验证。
