我来帮你更新 README，使其成为中英双语版本，并保持与当前代码（v5 存档格式、新机器等）同步。

---

# Chemical-World

[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![FTXUI](https://img.shields.io/badge/FTXUI-6.1.9-green.svg)](https://github.com/ArthurSonzogni/FTXUI)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)
[![Save Format](https://img.shields.io/badge/save-v5-green.svg)](#存档系统)

> A terminal-based interactive industrial sandbox game built with C++20 and FTXUI, inspired by a simplified power network and machine chain from GT:New Horizons. Mine ores → smelt in blast furnace → machine steel parts on lathe → sell for coins / assemble more machines. Build your industrial empire.
>
> 一个基于 C++20 和 FTXUI 构建的终端交互式工业沙盒游戏，灵感来自 GT:New Horizons 的简化电力网络与机器链。挖掘矿石 → 土高炉冶炼 → 车床加工钢零件 → 卖钱 / 装配更多机器，搭建自己的工业帝国。

---

## 🎮 Features / 游戏特色

| EN | 中文 |
|----|------|
| **Three maps**: Home 60×50 (rivers/lakes/flowers), Wasteland 150×80 (8 biomes), Cave 45×28 (3 types) | **三大地图**：家园 60×50（河流/湖泊/花草装饰）、荒原 150×80（8 种子群系）、矿洞 45×28（3 种子类型） |
| **Power system**: Thermal generators + wire network + shared EU pool (0~10000), machines auto-pause without power | **电力系统**：火力发电机 + 电线网络 + 共享 EU 池（0~10000），无电机器自动停摆 |
| **Blast Furnace**: 4-slot parallel smelting × 30s/batch × 24-frame ASCII animation (STOKING/HEATING/POURING), **no power required** | **土高炉**：4 槽并行冶炼 × 30s/批 × 24 帧 ASCII 动画（STOKING / HEATING / POURING 三阶段），**无需电力** |
| **Lathe**: 6 steel part molds, 2 EU/tick power drain, auto-pauses without power | **车床**：6 种钢零件模具，加工中每 tick 扣 2 EU，无电自动暂停并等待恢复 |
| **Processing chain**: Crusher (4 EU/slot) → Ore Washer (2 EU) → Centrifuge (8 EU), with byproducts | **处理链**：破碎机 (4 EU/槽) → 洗矿槽 (2 EU) → 离心机 (8 EU)，附带副产物 |
| **Build mode**: B key opens 7×7 zoom grid, place generators/wires/machines/decor | **建造模式**：B 键打开 7×7 放大网格，放置发电机 / 电线 / 装饰 |
| **Trading market**: T key opens SELL/BUY dual-tab interface. BUY sells raw materials/steel parts/machine blueprints | **交易市场**：T 键打开 SELL/BUY 双标签界面，BUY 出售原材料 / 钢零件 / 机器蓝图 |
| **Quest system**: Main/side quests with prerequisites, rewards, tracking, and claim mechanics | **任务系统**：主线/支线任务，前置条件、奖励、追踪与领取机制 |
| **Bank & investment**: Buy/sell Gold and Bitcoin with dynamic price fluctuations | **银行与投资**：买卖黄金和比特币，价格动态波动 |
| **Save system**: v5 binary format, backward compatible, 3 manual slots + autosave | **存档系统**：v5 二进制存档格式，向后兼容，3 个手动档位 + 自动档 |

---

## ⌨️ Key Mapping / 按键映射

| Key / 按键 | Function / 功能 |
|------------|-----------------|
| `W` `A` `S` `D` | Move / 移动 |
| `E` | Interact (mine / enter / use machine / open generator panel) / 交互（开采 / 进入 / 使用机器 / 打开发电机面板） |
| `B` | Open build mode (Home only) / 打开建造模式（仅在家园） |
| `C` | Open backpack (inventory) / 打开背包（物品栏） |
| `T` | Trading market (SELL / BUY tabs) / 交易市场（SELL / BUY 双标签） |
| `K` | Bank & investment desk / 银行与投资界面 |
| `J` | Quest book / 任务书 |
| `F` | Furnace location hint / 高炉位置提示 |
| `H` | Help / 帮助 |
| `P` | Save to current manual slot / 保存到当前手动档 |
| `L` | Load current manual slot / 读取当前手动档 |
| `Q` | Quit with autosave / 退出并自动保存 |
| `ESC` | Exit current FTXUI fullscreen interface / 退出当前 FTXUI 全屏界面 |
| `SPACE` | Track/untrack quest / 追踪/取消追踪任务 |
| `R` | Claim quest reward / 领取任务奖励 |
| `+` `-` | Zoom quest book / 缩放任务书 |

---

## 📁 Project Structure / 项目结构

```
Chemical-World/
├── Chemical-World.cpp              # Single-file game source (~4500 lines) / 单文件游戏源码（约 4500 行）
├── Chemical-World.vcxproj          # VS project config / VS 项目配置
├── chemical_world_slot1.sav        # Manual save slot 1 / 手动存档槽 1
├── chemical_world_slot2.sav        # Manual save slot 2 / 手动存档槽 2
├── chemical_world_slot3.sav        # Manual save slot 3 / 手动存档槽 3
├── chemical_world_autosave.sav     # Autosave (written on exit) / 自动存档（退出时写入）
├── quests.json                     # Quest definitions (loads built-in if missing) / 任务定义（缺失时加载内置）
├── quest_progress.json             # Quest progress tracking / 任务进度追踪
└── README.md                       # This file / 本文件
```

### Core Modules / 核心模块说明

| Module / 模块 | Code Location / 代码位置 | Description / 功能描述 |
|---------------|--------------------------|------------------------|
| **Game Loop** / 游戏主循环 | `ChemicalWorldGame::run()` | WASD/arrow input, state machine, render scheduling / WASD/方向键输入、状态机、渲染调度 |
| **Map System** / 地图系统 | `GameMap` class | Three areas, mineral generation, decorations, biome variants / 三区域，矿物生成、装饰、子群系变种 |
| **Blast Furnace** / 土高炉 | `BlastFurnace` class | 4-slot parallel × 30s × 24-frame animation, no power / 4 槽并行 × 30s × 24 帧动画，无电力 |
| **Lathe** / 车床 | `Lathe` class | 6 molds, 2 EU/tick power drain, pause without power / 6 模具，2 EU/tick 电力消耗，无电暂停 |
| **Processing Chain** / 处理链 | `Crusher` / `OreWasher` / `Centrifuge` classes | Crush → Wash → Centrifuge, with byproducts / 破碎 → 洗矿 → 离心，附带副产物 |
| **Power Network** / 电力网络 | `tickPowerGrid` / `powerDraw` / `PowerGenerator` | BFS conductivity, EU pool injection/drain / BFS 导电性、EU pool 注入/扣减 |
| **FTXUI Interfaces** / FTXUI 界面 | `FurnaceUI` / `LatheUI` / `TradeUI` / `BuildUI` / `GeneratorPanel` / `BankUI` | Fullscreen UI, Maybe gating, ticker threads / 全屏 UI、Maybe 门控、ticker 线程 |
| **Quest System** / 任务系统 | `QuestManager` class | JSON config, prerequisites, conditions, rewards, tracking / JSON 配置、前置条件、条件、奖励、追踪 |
| **Build System** / 建造系统 | `openBuildUI` / `placeGenerator` / `placeWire` / `placeDecor` / `placeCrusher` / `placeWasher` / `placeCentrifuge` | 7×7 zoom grid, blueprint validation, space check / 7×7 放大网格、蓝图校验、空间校验 |
| **Trading System** / 交易系统 | `TradeUI` class | SELL/BUY dual-tab, blueprints one-time unlock, price = sell × 3 / SELL/BUY 双标签、蓝图一次性解锁、定价 = 售价 × 3 |
| **Save System** / 存档系统 | `saveGame` / `loadGame` | v5 format (v2–v4 compatible), 3 manual + autosave / v5 格式（v2–v4 兼容）、3 手动档 + autosave |

---

## 🛠️ Build & Dependencies / 构建与依赖

### Requirements / 环境要求

| EN | 中文 |
|----|------|
| Compiler: MSVC 19.x (Visual Studio 2022) | 编译器：MSVC 19.x（Visual Studio 2022） |
| C++ Standard: C++20 | C++ 标准：C++20 |
| Platform: Windows 10/11 (console must support ANSI color) | 平台：Windows 10/11（控制台需支持 ANSI 颜色） |
| Build Tool: MSBuild or Visual Studio IDE | 构建工具：MSBuild 或 Visual Studio IDE |

### Dependencies / 依赖库

| Library / 库 | Version | Purpose / 用途 | Path / 路径 |
|--------------|---------|----------------|-------------|
| [FTXUI](https://github.com/ArthurSonzogni/FTXUI) | 6.1.9 (win64) | Terminal UI rendering / 终端 UI 渲染 | `D:\360Downloads\ftxui-6.1.9-win64` |
| [nlohmann/json](https://github.com/nlohmann/json) | (header-only) | JSON parsing for quests / 任务 JSON 解析 | `json.hpp` in project |
| Windows API | - | Console color / cursor / `_getch` / 控制台颜色 / 光标 / `_getch` | System / 系统 |

### Build Steps / 构建步骤

#### Method 1: Visual Studio IDE / 方式一：Visual Studio IDE
1. Clone the repository / 克隆仓库：
   ```bash
   git clone https://github.com/CodeJ-40404/Chemical-World.git
   cd Chemical-World
   ```
2. Download FTXUI 6.1.9 win64 pre-built package and extract to `D:\360Downloads\ftxui-6.1.9-win64` (or modify `.vcxproj` include/lib paths accordingly / 或修改 `.vcxproj` 中的包含/库路径)
3. Open `Chemical-World.slnx` or `Chemical-World.vcxproj` with VS 2022
4. Select `Debug` + `x64`, build solution / 选择 `Debug` + `x64`，生成解决方案

#### Method 2: Command Line MSBuild / 方式二：命令行 MSBuild
```powershell
& "D:\c++\for-vs\MSBuild\Current\Bin\MSBuild.exe" `
    "d:\c++\repos\Chemical-World\Chemical-World\Chemical-World.vcxproj" `
    /p:Configuration=Debug /p:Platform=x64 /v:minimal /m
```

Output: `d:\c++\repos\Chemical-World\Chemical-World\x64\Debug\Chemical-World.exe`

---

## 🧪 Contributing / 贡献指南

### Code Standards / 代码规范

| EN | 中文 |
|----|------|
| Naming: `PascalCase` for classes, `camelCase` for functions/variables, no prefix for private members | 命名：类名 `PascalCase`，函数/变量 `camelCase`，私有成员无前缀 |
| Comments: Chinese allowed, but in-game display text **must use English + ASCII** (to avoid terminal corruption) | 注释：中文注释允许，但游戏内显示文本请使用 **英文 + ASCII**（避免终端乱码） |
| Single-file constraint: All logic stays in `Chemical-World.cpp` (~4500 lines) | 单文件约束：所有逻辑写在 `Chemical-World.cpp` 一个文件里（约 4500 行） |
| FTXUI pattern: Avoid `[&]` capturing local vectors that Menu/Renderer then hold by reference — will dangle | FTXUI 模式：避免在 setupUI 局部 lambda 中 `[&]` 捕获局部 vector 然后让 Menu/Renderer 持有引用 —— 会悬空 |

### Submission Guidelines / 提交指南
1. Fork this project / Fork 本项目
2. Create a feature branch: `git checkout -b feature/your-awesome-feature` / 创建功能分支
3. Commit changes: `git commit -m "feat: added xxx"` / 提交变更
4. Push branch: `git push origin feature/your-feature` / 推送分支
5. Open a Pull Request describing changes and test results / 发起 Pull Request，描述变更内容与测试结果

### Testing Requirements / 测试要求
- At least MSBuild `EXIT=0` compilation pass / 至少保证 MSBuild `EXIT=0` 编译通过
- Game runs without crashes / 游戏能正常游玩/操作，不会莫名闪退
- Does not deviate too far from the project direction (hardcore industrial-chemistry-space sandbox) / 不过于偏离项目方向（工业-化学-宇航硬核沙盒）
- No serious environment errors / 没有严重的环境错误

---

## 🚀 Roadmap / 发展路线

### Completed / 已完成
- [x] Basic map rendering & movement (3 areas + biomes) / 基础地图渲染与移动（三区域 + 子群系）
- [x] Mineral generation & mining system (3-hit mechanics) / 矿物生成与开采系统（3 次敲击机制）
- [x] **Blast Furnace rewrite**: 4-slot × 30s × 24-frame ASCII animation, no power / **土高炉重写**：4 槽并行 × 30s × 24 帧 ASCII 动画，无需电力
- [x] **Lathe**: 6 molds + machining animation + power drain (pause without power) / **车床系统**：6 模具 + 加工动画 + 电力消耗（无电暂停）
- [x] **Processing chain**: Crusher / Washer / Centrifuge with byproducts / **处理链**：破碎机 / 洗矿槽 / 离心机，附带副产物
- [x] **Power network**: Thermal generators + wire BFS + EU pool (0~10000) / **电力网络**：火力发电机 + 电线 BFS + EU pool（0~10000）
- [x] **Build mode**: B key 7×7 zoom grid, generators/wires/machines/decor / **建造模式**：B 键 7×7 放大网格，发电机 / 电线 / 装饰
- [x] **Trading market**: SELL/BUY dual-tab + blueprints one-time unlock / **交易市场**：SELL/BUY 双标签 + 蓝图一次性解锁
- [x] **Quest system**: Main/side quests, prerequisites, tracking, claiming / **任务系统**：主线/支线任务、前置条件、追踪与领取
- [x] **Bank & investment**: Gold/BTC trading with dynamic prices / **银行与投资**：黄金/比特币交易，动态价格
- [x] **Save system**: v5 format (v2–v4 compatible) + 3 manual + autosave / **存档系统**：v5 格式（v2–v4 兼容）+ 3 手动档 + autosave
- [x] Experience & level system / 经验值与等级系统

### Planned / 规划中
- [ ] More machines: Compressor / Chemical Reactor / Electrolyzer / 更多机器：压缩机 / 化学反应釜 / 电解机
- [ ] Fluid pipes: Water / Oil / Chemical solution transport / 流体管道：水 / 油 / 化学溶液运输
- [ ] Multi-recipe extension: Dynamic recipe registration / 多配方扩展：动态配方注册机制
- [ ] AI merchants: NPC trading logic with price fluctuations / AI 商人：基于价格波动的 NPC 交易逻辑
- [ ] Cross-platform: Linux / macOS terminal support (replace Windows API) / 跨平台适配：Linux / macOS 终端支持（替换 Windows API）
- [ ] Unit tests: Google Test coverage for core logic / 单元测试：使用 Google Test 覆盖核心逻辑

### Long-term Vision / 长期愿景
To become a massive hardcore game integrating industry, chemistry, and space exploration. After the basic framework and game logic are complete, **we strive to keep it as magic-free as possible**.
>
> 成为一个庞大且融合工业-化学-宇航的硬核游戏，在基础框架与游戏逻辑完工后，**我们尽可能保证没有魔法合成**。

---

## 📜 License / 许可证

[MIT License](LICENSE.txt) © 2026 [CodeJ-40404](https://github.com/CodeJ-40404)

---

## 📌 Game Tutorial / 关于游戏教程

**README does not contain detailed gameplay tutorials.** For more information, please refer to:
>
> **README 不包含详细玩法教程。** 如需了解具体内容，请查阅：

- **In-game help**: Press `H` to view key bindings / **游戏内帮助**：按 `H` 键查看按键列表
- **Project Wiki**: [Chemical-World Wiki](https://github.com/CodeJ-40404/Chemical-World/wiki) (in the `wiki/` directory of this repo / 本仓库 `wiki/` 目录下)
  - [Home](https://github.com/CodeJ-40404/Chemical-World/wiki) — Wiki home & index / Wiki 首页与索引
  - [Gameplay-Basics](https://github.com/CodeJ-40404/Chemical-World/wiki/Gameplay-Basics) — Gameplay basics / 玩法基础
  - [Power-System](https://github.com/CodeJ-40404/Chemical-World/wiki/Power-System) — Power system details / 电力系统详解
  - [Machines](https://github.com/CodeJ-40404/Chemical-World/wiki/Machines) — Machines (Furnace/Lathe/Generator/Processing) / 机器（高炉/车床/发电机/处理链）
  - [Trading-Market](https://github.com/CodeJ-40404/Chemical-World/wiki/Trading-Market) — Trading market / 交易市场
  - [Build-Mode](https://github.com/CodeJ-40404/Chemical-World/wiki/Build-Mode) — Build mode / 建造模式
  - [Quest-System](https://github.com/CodeJ-40404/Chemical-World/wiki/Quest-System) — Quest system / 任务系统
  - [Save-System](https://github.com/CodeJ-40404/Chemical-World/wiki/Save-System) — Save system / 存档系统
  - [Controls](https://github.com/CodeJ-40404/Chemical-World/wiki/Controls) — Complete key bindings / 按键完整列表

---

## 🤝 Contact / 联系方式

- Bug reports: [GitHub Issues](https://github.com/CodeJ-40404/Chemical-World/issues) / 问题反馈
- Discussion: Discord (not yet) / 讨论交流：Discord 群组（暂时没有）
- Maintainer: [@CodeJ-40404](https://github.com/CodeJ-40404) / 维护者

---

*Happy Gaming! / 祝玩得开心！* 🧪⚗️⚡
