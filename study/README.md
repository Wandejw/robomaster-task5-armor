# Task5 学习代码与题目

## 已收录的练习

这里只表示目录中存在对应代码，不表示已经通过正确性验收或完成独立掌握检查。

| 题号 | 题目 | 练习代码 |
|---|---|---|
| 1 | 通道交换 | [q1](算法代码/q1) |
| 2 | 灰度化 | [q2](算法代码/q2) |
| 3 | 二值化 | [q3](算法代码/q3) |
| 5 | HSV 变换与颜色反转 | [q5](算法代码/q5) |
| 7 | 平均池化 | [q7](算法代码/q7) |
| 9 | 高斯滤波 | [q9](算法代码/q9) |
| 10 | 中值滤波 | [q10](算法代码/q10) |

上述题目的题面、输入图和参考结果见 [问题一至问题十](ImageProcessing100Wen/Question_01_10/README.md)。各练习保留原始源码，不替换为题库参考答案。

## 编译与运行

每道题是独立的 CMake 项目。例如从仓库根目录构建 q9：

```bash
cmake -S study/算法代码/q9 -B study/算法代码/q9/build
cmake --build study/算法代码/q9/build
```

程序通过相对路径 `input.jpg` 读取图片，运行时应在相应练习目录中执行。q9、q10 已附输入图片；其余练习需先准备 `input.jpg`，可使用原题目录中的 `imori.jpg`。可执行文件名和输出位置以各题的 `CMakeLists.txt` 为准。

## 题库与离线平台

- [ImageProcessing100Wen](ImageProcessing100Wen/README.md)：原始题目、素材及参考代码。
- [Task5 OpenCV 精选题平台](Task5_OpenCV平台/README.md)：23 道精选题，打开其 `index.html` 使用。
- 平台内保留独立题库副本，供现有数据生成脚本使用。

题库来源为 gzr2017/ImageProcessing100Wen，保留其 MIT [许可](ImageProcessing100Wen/LICENSE)；平台内副本也保留原许可。题库参考代码与 `算法代码/` 中的个人练习分别存放。

本目录未收录构建产物、缓存、旧备份或嵌套 Git 历史。浏览器 localStorage 中的学习状态与笔记不属于这些文件，不会随仓库一起同步。
