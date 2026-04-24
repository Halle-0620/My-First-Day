# My First Day

《第一天》是一个基于 Qt Widgets 的叙事驱动互动体验 Demo，采用接近视觉小说的形式呈现。

## 当前状态

- 已完成数据驱动叙事骨架
- 主线场景已接入到场景 12
- 支线 7B 和极简梦境模块已接入
- 当前重点是文本流程、变量承接和交互验证

## 技术栈

- C++
- Qt 6 Widgets
- CMake
- JSON 数据驱动叙事内容

## 目录结构

- `src/`：Qt UI、叙事引擎和运行时代码
- `data/`：叙事内容数据文件
- `build/`：本地构建产物，不纳入版本控制

## 本地构建

在 Windows PowerShell 下执行：

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/mingw_64" -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe"
cmake --build build
.\build\MyFirstDay.exe
```

## 环境变量

当前项目本地构建不依赖必填环境变量。后续如果加入外部服务配置，请参考 `.env.example`。
