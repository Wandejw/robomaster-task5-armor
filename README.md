# Task5 · 装甲板识别

RobotPilots 算法组训练项目，使用 C++ 和 OpenCV，从视频中提取灯条并配对生成装甲板候选框。

## 功能与训练内容

- HSV 颜色分割与形态学处理，提取红色、蓝色候选区域。
- 根据尺寸、长宽比、填充率和倾角筛选灯条。
- 根据方向、长度和相对位置配对同色灯条，并结合前帧结果调整配对优先级。
- 提供暂停、进度跳转、逐帧查看和参数滑块，辅助分析漏检与错配。
- 附带 [学习代码与对应题目](study/README.md)：7 份练习代码、原题库和 23 道精选题的离线学习平台。

## 运行

依赖 C++17、CMake 和 OpenCV。

```bash
cmake -S . -B build
cmake --build build
./build/task5_armor
```

预览与调参程序：

```bash
cmake -S preview -B preview/build
cmake --build preview/build
./preview/build/test
```

运行前将 `main/main.cpp` 或 `preview/preview.cpp` 中的视频路径改为自己的文件路径；视频未纳入仓库。红框表示灯条候选，黄框表示装甲板配对结果。

包含主项目与学习材料的固定版本标记为 [`task5`](https://github.com/Wandejw/robomaster-task5-armor/tree/task5)。
