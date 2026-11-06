# Task5 · RoboMaster 装甲板识别与 OpenCV 学习

本仓库收录 Task5 装甲板识别项目，以及完成任务期间使用的学习题目、练习代码和离线学习平台。

- 装甲板识别程序：根目录下的 `main/`、`processing/`、`detection/`、`matching/` 等目录。
- 识别与调参预览程序：`preview/`。
- [学习代码、题目与运行说明](study/README.md)。
- [离线精选题平台说明](study/Task5_OpenCV平台/README.md)。

依赖 C++17、CMake 和 OpenCV。主项目与预览程序分别构建：

```bash
cmake -S . -B build
cmake --build build
cmake -S preview -B preview/build
cmake --build preview/build
```

视频文件未纳入仓库。主程序和预览程序中的视频路径目前为原机器的绝对路径，运行前需要改为自己的视频位置。

`task5` Git 标签标记包含主项目和附带学习材料的完整版本。
