# Chemical-World

[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![FTXUI](https://img.shields.io/badge/FTXUI-6.1.9-green.svg)](https://github.com/ArthurSonzogni/FTXUI)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)
[![Save Format](https://img.shields.io/badge/save-v3-orange.svg)](#存档系统)

一个基于 C++20 和 FTXUI 构建的终端交互式工业沙盒游戏，灵感来自 [GT:New Horizons](https://www.curseforge.com/minecraft/modpacks/gt-new-horizons) 的简化电力网络与机器链。挖掘矿石 → 土高炉冶炼 → 车床加工钢零件 → 卖钱 / 装配更多机器，搭建自己的工业帝国。

---

## 🎮 游戏特色

- **三大地图**：家园 60×50（含河流 / 湖泊 / 花草装饰）、荒原 150×80、矿洞 45×28
- **电力系统**：火力发电机 + 电线网络 + 共享 EU 池（0~10000），无电机器自动停摆
- **土高炉**：4 槽并行冶炼 × 30s/批 × 24 帧 ASCII 动画（STOKING / HEATING / POURING 三阶段），**无需电力**
- **车床**：6 种钢零件模具，加工中每 tick 扣 2 EU，无电自动暂停并等待恢复
- **建造模式**：B 键打开 7×7 放大网格，放置发电机 / 电线 / 装饰
- **交易市场**：T 键打开 SELL/BUY 双标签界面，BUY 出售原材料 / 钢零件 / 机器蓝图
- **存档系统**：v3 二进制存档格式，向后兼容 v2，3 个手动档位 + 自动档

---

## ⌨️ 按键映射

| 按键 | 功能 |
|------|------|
| `W` `A` `S` `D` | 移动 |
| `E` | 交互（开采 / 进入 / 使用机器 / 打开发电机面板） |
| `B` | 打开建造模式（仅在家园） |
| `C` | 合成菜单 |
| `T` | 交易市场（SELL / BUY 双标签） |
| `F` | 高炉位置提示 |
| `H` | 帮助 |
| `P` | 保存到当前手动档 |
| `L` | 读取当前手动档 |
| `Q` | 退出并自动保存到 autosave |
| `ESC` | 退出当前 FTXUI 全屏界面 |

---

## 📁 项目结构

```
Chemical-World/
├── Chemical-World.cpp              # 单文件游戏源码（约 3100 行）
├── Chemical-World.vcxproj          # VS 项目配置 (Debug|x64, C++20)
├── chemical_world_slot1.sav        # 手动存档槽 1
├── chemical_world_slot2.sav        # 手动存档槽 2
├── chemical_world_slot3.sav        # 手动存档槽 3
├── chemical_world_autosave.sav     # 自动存档（退出时写入）
└── README.md                       # 本文件
```

### 核心模块说明

| 模块 | 代码位置 | 功能描述 |
|------|----------|----------|
| **游戏主循环** | `ChemicalWorldGame::run()` | WASD/方向键输入、状态机、渲染调度 |
| **地图系统** | `GameMap` 类 | 60×50/150×80/45×28 三区域，矿物生成、装饰、存档 |
| **土高炉** | `BlastFurnace` 类 | 4 槽并行 × 30s × 24 帧动画，无电力需求 |
| **车床** | `Lathe` 类 | 6 模具，2 EU/tick 电力消耗，无电暂停 |
| **电力网络** | `tickPowerGrid` / `powerDraw` / `PowerGenerator` | BFS 导电性、EU pool 注入/扣减 |
| **FTXUI 界面** | `FurnaceUI` / `LatheUI` / `TradeUI` / `BuildUI` / `GeneratorPanel` | 全屏 UI、Maybe 门控、ticker 线程 |
| **建造系统** | `openBuildUI` / `placeGenerator` / `placeWire` / `placeDecor` | 7×7 放大网格、蓝图校验、空间校验 |
| **交易系统** | `TradeUI` 类 | SELL/BUY 双标签、蓝图一次性解锁、定价 = 售价 × 3 |
| **存档系统** | `saveGame` / `loadGame` | v3 格式（v2 兼容）、3 手动档 + autosave |

---

## 🛠️ 构建与依赖

### 环境要求
- **编译器**：MSVC 19.x（Visual Studio 2022）
- **C++ 标准**：C++20
- **平台**：Windows 10/11（控制台需支持 ANSI 颜色）
- **构建工具**：MSBuild 或 Visual Studio IDE

### 依赖库
| 库名称 | 版本 | 用途 | 路径 |
|--------|------|------|------|
| [FTXUI](https://github.com/ArthurSonzogni/FTXUI) | 6.1.9 (win64) | 终端 UI 渲染（Menu/Radiobox/Button/Renderer） | `D:\360Downloads\ftxui-6.1.9-win64` |
| Windows API | - | 控制台颜色 / 光标 / `_getch` | 系统 |

### 构建步骤

#### 方式一：Visual Studio IDE
1. 克隆仓库：
   ```bash
   git clone https://github.com/CodeJ-40404/Chemical-World.git
   cd Chemical-World
   ```
2. 下载 FTXUI 6.1.9 win64 预编译包，解压到 `D:\360Downloads\ftxui-6.1.9-win64`（或修改 `.vcxproj` 中的包含/库路径）
3. 用 VS 2026 打开 `Chemical-World.slnx` 或 `Chemical-World.vcxproj`
4. 选择 `Debug` + `x64`，生成解决方案

#### 方式二：命令行 MSBuild
```powershell
& "D:\c++\for-vs\MSBuild\Current\Bin\MSBuild.exe" `
    "d:\c++\repos\Chemical-World\Chemical-World\Chemical-World.vcxproj" `
    /p:Configuration=Debug /p:Platform=x64 /v:minimal /m
```

产物：`d:\c++\repos\Chemical-World\Chemical-World\x64\Debug\Chemical-World.exe`

---

## 🧪 贡献指南

### 代码规范
- **命名**：类名 `PascalCase`，函数 / 变量 `camelCase`，私有成员无前缀
- **注释**：中文注释允许，但游戏内显示文本请使用 **英文 + ASCII**（避免终端乱码）
- **单文件约束**：当前所有逻辑写在 `Chemical-World.cpp` 一个文件里（约 3100 行），新增功能尽量保持此结构
- **FTXUI 模式**：避免在 setupUI 局部 lambda 中 `[&]` 捕获局部 vector 然后让 Menu/Renderer 持有引用 —— 会悬空

### 提交指南
1. Fork 本项目
2. 创建功能分支：`git checkout -b feature/your-awesome-features`
3. 提交变更：`git commit -m "feat: 增加了xxx"`
4. 推送分支：`git push origin feature/your-feature`
5. 发起 Pull Request，描述变更内容与测试结果

### 测试要求
- 至少保证 MSBuild `EXIT=0` 编译通过
- 游戏能正常游玩 / 操作，不会莫名闪退
- 不过于偏离项目方向（工业-化学-宇航硬核沙盒）
- 没有严重的环境错误

---

## 🚀 项目发展路线

### 已完成
- [x] 基础地图渲染与移动（三区域 + 河流 / 湖泊 / 花草装饰）
- [x] 矿物生成与开采系统（3 次敲击机制）
- [x] **土高炉重写**：4 槽并行 × 30s × 24 帧 ASCII 动画，无需电力
- [x] **车床系统**：6 模具 + 加工动画 + 电力消耗（无电暂停）
- [x] **电力网络**：火力发电机 + 电线 BFS 连通 + EU pool（0~10000）
- [x] **建造模式**：B 键 7×7 放大网格，放置发电机 / 电线 / 装饰
- [x] **交易市场**：SELL/BUY 双标签 + 蓝图一次性解锁
- [x] **存档系统**：v3 格式（v2 兼容）+ 3 手动档 + autosave
- [x] 经验值与等级系统

### 规划中（Roadmap）
- [ ] **更多机器**：压缩机 / 化学反应釜 / 离心机
- [ ] **流体管道**：水 / 油 / 化学溶液运输
- [ ] **多配方扩展**：动态配方注册机制
- [ ] **AI 商人**：基于价格波动的 NPC 交易逻辑
- [ ] **跨平台适配**：Linux / macOS 终端支持（替换 Windows API）
- [ ] **单元测试**：使用 Google Test 覆盖核心逻辑

### 长期愿景
成为一个庞大且融合工业-化学-宇航的硬核游戏，在基础框架与游戏逻辑完工后，**我们尽可能保证没有魔法合成**。

---

## 📜 许可证

[MIT License](LICENSE.txt) © 2026 [CodeJ-40404](https://github.com/CodeJ-40404)

---

## 📌 关于游戏教程

**README 不包含详细玩法教程。** 如需了解具体内容，请查阅：

- **游戏内帮助**：按 `H` 键查看按键列表
- **项目 Wiki**：[Chemical-World Wiki](https://github.com/CodeJ-40404/Chemical-World/wiki)（本仓库 `wiki/` 目录下）
  - [Home](https://github.com/CodeJ-40404/Chemical-World/wiki) — Wiki 首页与索引
  - [Gameplay-Basics](https://github.com/CodeJ-40404/Chemical-World/wiki/Gameplay-Basics) — 玩法基础
  - [Power-System](https://github.com/CodeJ-40404/Chemical-World/wiki/Power-System) — 电力系统详解
  - [Machines](https://github.com/CodeJ-40404/Chemical-World/wiki/Machines) — 机器（高炉 / 车床 / 发电机）
  - [Trading-Market](https://github.com/CodeJ-40404/Chemical-World/wiki/Trading-Market) — 交易市场
  - [Build-Mode](https://github.com/CodeJ-40404/Chemical-World/wiki/Build-Mode) — 建造模式
  - [Save-System](https://github.com/CodeJ-40404/Chemical-World/wiki/Save-System) — 存档系统
  - [Controls](https://github.com/CodeJ-40404/Chemical-World/wiki/Controls) — 按键完整列表

---

## 🤝 联系方式

- 问题反馈：[GitHub Issues](https://github.com/CodeJ-40404/Chemical-World/issues)
- 讨论交流：Discord 群组（暂时没有）
- 维护者：[@CodeJ-40404](https://github.com/CodeJ-40404)

---

*Happy Gaming!* 🧪⚗️⚡
