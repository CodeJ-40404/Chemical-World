# Chemical-World 贡献指南 / Contribution Guide

[English](#english) | [中文](#chinese)

---

## 中文版

感谢你对 Chemical-World 的兴趣！这份指南将帮助你理解项目结构、开发流程和贡献规范。

---

### 📋 目录

1. [项目概况](#项目概况)
2. [环境搭建](#环境搭建)
3. [代码规范](#代码规范)
4. [开发工作流](#开发工作流)
5. [提交规范](#提交规范)
6. [测试要求](#测试要求)
7. [常见问题](#常见问题)

---

### 项目概况

#### 技术栈

| 组件 | 技术 | 说明 |
|------|------|------|
| 语言 | C++20 | 使用 `std::` 标准库，禁用 C 风格 `printf`/`scanf` |
| UI 框架 | FTXUI 6.1.9 | 终端渲染（Menu / Radiobox / Button / Renderer） |
| 平台 | Windows 10/11 | 依赖 `windows.h` 控制台 API（颜色/光标） |
| 构建 | MSVC 2022 | VS 项目 `.vcxproj` + MSBuild |
| 序列化 | 自定义二进制格式 | v3/v4/v5 向后兼容 |
| 存档扩展 | JSON（任务进度） | 使用 `nlohmann/json` |

#### 核心设计原则

1. **单文件架构**：所有游戏逻辑集中于 `Chemical-World.cpp`（~3100 行），便于分发和编译
2. **FTXUI 全屏模式**：所有复杂界面均使用 `ScreenInteractive::Fullscreen()` + `Loop()`
3. **100ms 心跳 tick**：`globalTick100ms()` 驱动机器进度、电网刷新、市场更新
4. **存档向后兼容**：新增字段时保留旧版读取逻辑，版本号递增

---

### 环境搭建

#### 前置条件

- **操作系统**：Windows 10/11（控制台需支持 ANSI 转义序列）
- **编译器**：Visual Studio 2022（MSVC 19.x）
- **C++ 标准**：C++20
- **依赖库**：FTXUI 6.1.9（预编译 win64 版本）

#### 获取 FTXUI

1. 从 [FTXUI Releases](https://github.com/ArthurSonzogni/FTXUI/releases) 下载 `ftxui-6.1.9-win64.zip`
2. 解压到本地目录（例如 `D:\360Downloads\ftxui-6.1.9-win64`）
3. 确保项目 `.vcxproj` 中的 `AdditionalIncludeDirectories` 和 `AdditionalLibraryDirectories` 指向该路径

#### 编译项目

**方式一：Visual Studio IDE**

```bash
git clone https://github.com/CodeJ-40404/Chemical-World.git
cd Chemical-World
# 用 VS 2022 打开 .slnx 或 .vcxproj
# 选择 Debug + x64 → 生成
```

**方式二：命令行 MSBuild**

```powershell
& "D:\c++\for-vs\MSBuild\Current\Bin\MSBuild.exe" `
    "d:\c++\repos\Chemical-World\Chemical-World\Chemical-World.vcxproj" `
    /p:Configuration=Debug /p:Platform=x64 /v:minimal /m
```

#### 验证环境

编译成功后，运行 `Chemical-World.exe`，应看到启动画面和存档选择界面。

---

### 代码规范

#### 命名约定

| 类型 | 风格 | 示例 |
|------|------|------|
| 类 / 结构体 | `PascalCase` | `BlastFurnace`, `QuestManager` |
| 函数 / 方法 | `camelCase` | `loadMaterials()`, `getStatus()` |
| 变量（局部） | `camelCase` | `selectedSlot`, `statusMessage` |
| 变量（成员） | `camelCase`（无前缀） | `player`, `gameMap` |
| 常量 / 枚举 | `PascalCase` 或 `UPPER_SNAKE` | `QuestState::Completed`, `EU_MAX` |
| 宏定义 | `UPPER_SNAKE` | `COLOR_RED`, `_CRT_SECURE_NO_WARNINGS` |

#### 注释规范

- **中文注释允许**，用于说明代码逻辑和设计意图
- **游戏内显示文本**必须使用 **英文 + 基础 ASCII**（避免终端乱码）
- **接口注释**：公共函数建议添加注释说明用途、参数和返回值

```cpp
// 土高炉：4 槽并行，不耗电，每槽 30s（300 × 100ms ticks）。
// 动画 24 帧：0-5 STOKING, 6-17 HEATING, 18-23 POURING
class BlastFurnace { /* ... */ };
```

#### 代码风格

- 使用 `#pragma once` 或 `#include` 守卫（本项目单文件，无需头文件）
- 缩进：4 空格（VS 默认）
- 大括号：K&R 风格（左括号不换行）

```cpp
if (condition) {
    doSomething();
} else {
    doOther();
}
```

#### FTXUI 开发注意事项

1. **避免悬空引用**：`Menu` / `Renderer` 的 lambda 中不要 `[&]` 捕获局部 `vector<string>`，因为 Renderer 会持有引用，函数返回后引用失效，点击菜单会崩溃。

```cpp
// ❌ 错误：buyLabels 在函数返回后析构
void setupUI() {
    vector<string> buyLabels = { "item1", "item2" };
    auto menu = Menu(&buyLabels, &selected);  // 悬空引用！
}

// ✅ 正确：将数据存储为类成员
class TradeUI {
    vector<string> buyLabels;  // 成员变量
    void setupUI() {
        buyLabels = { "item1", "item2" };
        auto menu = Menu(&buyLabels, &selected);
    }
};
```

2. **Maybe 门控**：多个状态互斥的按钮用 `Maybe(child, condition)` 包裹，避免幽灵按钮占用焦点。

3. **ticker 线程**：全屏 UI 中需要后台更新时，使用 `std::thread` 循环调用 `globalTick100ms()` + `screen.PostEvent(Event::Custom)`，退出时 `.join()`。

---

### 开发工作流

#### 分支策略

```
main
├── feature/xxx        # 新功能分支
├── fix/xxx            # Bug 修复分支
└── docs/xxx           # 文档分支（仅 README/wiki）
```

- 所有 PR 合并到 `main`
- 分支名称使用 `kebab-case`，例如 `feature/add-compressor-machine`

#### 功能开发流程

1. **创建分支**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **编写代码**：遵循[代码规范](#代码规范)

3. **自测**：
   - 编译通过（MSBuild `EXIT=0`）
   - 游戏基本可玩（进入、移动、打开界面、保存/读取）
   - 新增功能的核心路径至少走一遍

4. **更新文档**：
   - 如有新按键，更新 `README.md` 按键表
   - 如有新机制，更新 `wiki/` 对应页面

5. **提交代码**（参见[提交规范](#提交规范)）

6. **发起 Pull Request**

#### 新增机器/配方的标准步骤

1. **数据结构**：在 `struct` 区域定义机器类（如 `Crusher` / `OreWasher`）
2. **配方表**：在机器类内定义 `Recipe` 结构体和 `recipes` 向量
3. **UI 面板**：实现对应的 `openXxxUI()` 方法（参考 `openCrusherUI`）
4. **建造集成**：在 `openBuildUI()` 的 `placeEntries` 中添加选项，实现 `placeXxx()` 方法
5. **存档集成**：在 `saveGame()` 和 `loadGame()` 中添加机器元数据
6. **电网集成**：在 `tickPowerGrid()` 的导电性判断中加入新机器字符
7. **E 键交互**：在 `processKey` 的 `'E'` 分支添加 `findNearbyMachine('X', ...)` 检测

---

### 提交规范

#### Commit Message 格式

```
<type>(<scope>): <subject>

[body]

[footer]
```

**Type（必填）**

| 类型 | 说明 |
|------|------|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `docs` | 文档更新（README / wiki） |
| `style` | 代码格式（缩进、空格） |
| `refactor` | 重构（不改变功能） |
| `test` | 测试相关 |
| `chore` | 构建/工具链变动 |

**Scope（可选）**：模块名，如 `furnace`、`lathe`、`power`、`build`、`save`

**Subject**：简短描述，不超过 50 字符，使用祈使句

**Body（可选）**：详细说明，每行不超过 72 字符

**Footer（可选）**：关联 Issue，如 `Closes #42`

#### 示例

```
feat(crusher): add 4-slot ore crusher machine

- Implement Crusher class with 4 parallel slots
- Add openCrusherUI() FTXUI panel
- Integrate with build mode and power grid
- Support 8 ore types -> crushed ore + gravel

Closes #17
```

```
fix(lathe): prevent crash when loading saved game without lathe meta

Lathe position was hardcoded to (8,5), causing out-of-bounds access
when game is loaded from a save without lathe machineMeta entry.

Now dynamically find lathe from machineMeta on each tick.
```

---

### 测试要求

#### 编译测试

- **MSBuild 必须返回 0**（无编译错误/警告）
- 警告级别 `/W3`，新代码不应引入新的警告

#### 功能测试（手动）

提交 PR 前，请至少验证以下场景：

| 模块 | 测试项 |
|------|--------|
| 启动 | 进入存档选择界面 → 新游戏/加载存档正常 |
| 移动 | WASD / 方向键移动，边界碰撞正确 |
| 采矿 | E 键附近矿物，3 次敲击获得矿石 |
| 高炉 | F 键提示，E 键打开面板，加载/冶炼/收集正常 |
| 车床 | E 键打开面板，选择模具/加载/加工（有电/无电）正常 |
| 电网 | 发电机投煤 → 铺电线 → 车床通电正常；断电自动暂停 |
| 建造 | B 键打开面板，放置发电机/电线/装饰，校验生效 |
| 交易 | T 键打开，SELL/BUY 切换，买卖/蓝图解锁正常 |
| 银行 | K 键打开，资产价格更新，买卖正常 |
| 任务 | J 键打开，任务进度追踪正常 |
| 存档 | P 保存 → 退出 → 重新加载，进度完整恢复 |
| 跨区 | Car → 荒原 → 洞穴 → 返回家园，地图切换正常 |

#### 回归测试

若修改了核心模块（存档、电网、机器进度），建议测试旧存档兼容性：

1. 用旧版本生成一个存档（`chemical_world_slot1.sav`）
2. 切换到你修改后的版本加载
3. 检查关键数据（玩家位置、物品、机器状态）是否完整恢复

---

### 常见问题

#### Q1: 编译时找不到 FTXUI 头文件

**A**: 检查 `.vcxproj` 中的包含路径，确认指向 FTXUI 的 `include/` 目录。例如：
```xml
<AdditionalIncludeDirectories>
    D:\360Downloads\ftxui-6.1.9-win64\include;
    %(AdditionalIncludeDirectories)
</AdditionalIncludeDirectories>
```

#### Q2: 运行时 FTXUI 界面显示乱码/空白

**A**: 确保终端支持 UTF-8 和 ANSI 颜色。在 Windows Terminal 或 VS Code 终端中运行；不推荐使用 CMD 旧版。

#### Q3: 在 FTXUI 界面中按 Enter 没反应（按钮不可见）

**A**: 检查按钮是否被 `Maybe` 门控条件遮蔽，或者按钮未加入 `Container` 的焦点链。常见错误：
```cpp
// ❌ 错误：buttons 是临时对象，未赋值给变量
Renderer(Container::Vertical({ buttonA, buttonB }), [&] { ... });

// ✅ 正确
auto buttons = Container::Vertical({ buttonA, buttonB });
Renderer(buttons, [&] { ... });
```

#### Q4: 存档加载后机器位置丢失（幽灵方块）

**A**: 地图 `generate()` 会重置 tile，但 `machineMeta` 保留了元数据。调用 `rehydrateMachineTiles()` 从 meta 重建 tile 显示。在 `loadGame()` 末尾已自动调用。

#### Q5: 如何添加新的机器字符到电网 BFS 导电性？

**A**: 在 `tickPowerGrid()` 的 `isConductive` lambda 中添加：
```cpp
if (d == 'X' || d == 'x' || d == 'W' || d == 'w' || d == 'R' || d == 'r') return true;
```

---

再次感谢你的贡献！愿你的代码 🧪 稳定，⚡ 高效，🚀 永无 bug。

---

---

## IN English

Thank you for your interest in Chemical-World! This guide will help you understand the project structure, development workflow, and contribution guidelines.

---

### 📋 Table of Contents

1. [Project Overview](#project-overview)
2. [Environment Setup](#environment-setup)
3. [Code Standards](#code-standards)
4. [Development Workflow](#development-workflow)
5. [Commit Guidelines](#commit-guidelines)
6. [Testing Requirements](#testing-requirements)
7. [FAQ](#faq)

---

### Project Overview

#### Tech Stack

| Component | Technology | Description |
|-----------|------------|-------------|
| Language | C++20 | Uses `std::` library; C-style `printf`/`scanf` are disallowed |
| UI Framework | FTXUI 6.1.9 | Terminal rendering (Menu / Radiobox / Button / Renderer) |
| Platform | Windows 10/11 | Depends on `windows.h` console API (colors/cursor) |
| Build System | MSVC 2022 | VS project `.vcxproj` + MSBuild |
| Serialization | Custom binary format | v3/v4/v5 backward compatible |
| Save Extension | JSON (quest progress) | Uses `nlohmann/json` |

#### Core Design Principles

1. **Single-file Architecture**: All game logic in `Chemical-World.cpp` (~3100 lines) for easy distribution and compilation
2. **FTXUI Fullscreen Mode**: All complex UI uses `ScreenInteractive::Fullscreen()` + `Loop()`
3. **100ms Heartbeat Tick**: `globalTick100ms()` drives machine progress, grid refresh, and market updates
4. **Backward Compatible Saves**: Keep old version read logic when adding new fields; increment version number

---

### Environment Setup

#### Prerequisites

- **OS**: Windows 10/11 (console must support ANSI escape sequences)
- **Compiler**: Visual Studio 2022 (MSVC 19.x)
- **C++ Standard**: C++20
- **Dependency**: FTXUI 6.1.9 (precompiled win64)

#### Getting FTXUI

1. Download `ftxui-6.1.9-win64.zip` from [FTXUI Releases](https://github.com/ArthurSonzogni/FTXUI/releases)
2. Extract to a local directory (e.g., `D:\360Downloads\ftxui-6.1.9-win64`)
3. Ensure `.vcxproj` `AdditionalIncludeDirectories` and `AdditionalLibraryDirectories` point to this path

#### Building the Project

**Option 1: Visual Studio IDE**

```bash
git clone https://github.com/CodeJ-40404/Chemical-World.git
cd Chemical-World
# Open .slnx or .vcxproj with VS 2022
# Select Debug + x64 → Build
```

**Option 2: Command-line MSBuild**

```powershell
& "D:\c++\for-vs\MSBuild\Current\Bin\MSBuild.exe" `
    "d:\c++\repos\Chemical-World\Chemical-World\Chemical-World.vcxproj" `
    /p:Configuration=Debug /p:Platform=x64 /v:minimal /m
```

#### Verify Environment

After successful build, run `Chemical-World.exe`. You should see the splash screen and save slot selection.

---

### Code Standards

#### Naming Conventions

| Type | Style | Example |
|------|-------|---------|
| Classes / Structs | `PascalCase` | `BlastFurnace`, `QuestManager` |
| Functions / Methods | `camelCase` | `loadMaterials()`, `getStatus()` |
| Variables (local) | `camelCase` | `selectedSlot`, `statusMessage` |
| Variables (member) | `camelCase` (no prefix) | `player`, `gameMap` |
| Constants / Enums | `PascalCase` or `UPPER_SNAKE` | `QuestState::Completed`, `EU_MAX` |
| Macros | `UPPER_SNAKE` | `COLOR_RED`, `_CRT_SECURE_NO_WARNINGS` |

#### Comment Guidelines

- **Chinese comments allowed** for code logic and design intent
- **In-game display text** must use **English + basic ASCII** (avoid terminal garbling)
- **Interface comments**: Public functions should have comments describing purpose, parameters, and return values

```cpp
// Blast furnace: 4 parallel slots, no power required, 30s per batch (300 × 100ms ticks).
// Animation 24 frames: 0-5 STOKING, 6-17 HEATING, 18-23 POURING
class BlastFurnace { /* ... */ };
```

#### Code Style

- Use `#pragma once` or `#include` guards (single-file project, no headers needed)
- Indentation: 4 spaces (VS default)
- Braces: K&R style (opening brace on same line)

```cpp
if (condition) {
    doSomething();
} else {
    doOther();
}
```

#### FTXUI Development Notes

1. **Avoid Dangling References**: Don't use `[&]` to capture local `vector<string>` in `Menu`/`Renderer` lambdas. The Renderer holds a reference that becomes invalid after the function returns, causing a crash when clicking the menu.

```cpp
// ❌ Wrong: buyLabels destructs after setupUI() returns
void setupUI() {
    vector<string> buyLabels = { "item1", "item2" };
    auto menu = Menu(&buyLabels, &selected);  // Dangling reference!
}

// ✅ Correct: store data as class member
class TradeUI {
    vector<string> buyLabels;  // Member variable
    void setupUI() {
        buyLabels = { "item1", "item2" };
        auto menu = Menu(&buyLabels, &selected);
    }
};
```

2. **Maybe Guards**: Use `Maybe(child, condition)` for mutually exclusive buttons to prevent ghost buttons from stealing focus.

3. **Ticker Thread**: When background updates are needed in fullscreen UI, use `std::thread` loop calling `globalTick100ms()` + `screen.PostEvent(Event::Custom)`, and `.join()` on exit.

---

### Development Workflow

#### Branch Strategy

```
main
├── feature/xxx        # New feature branch
├── fix/xxx            # Bug fix branch
└── docs/xxx           # Documentation branch (README/wiki only)
```

- All PRs merge into `main`
- Branch names use `kebab-case`, e.g., `feature/add-compressor-machine`

#### Feature Development Process

1. **Create branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Write code**: Follow [Code Standards](#code-standards)

3. **Self-test**:
   - Build succeeds (MSBuild `EXIT=0`)
   - Game is basically playable (enter, move, open UI, save/load)
   - Core path of new feature works at least once

4. **Update documentation**:
   - If new keys added, update `README.md` key table
   - If new mechanics added, update corresponding `wiki/` page

5. **Commit code** (see [Commit Guidelines](#commit-guidelines))

6. **Open Pull Request**

#### Standard Steps for Adding New Machines/Recipes

1. **Data Structure**: Define machine class in `struct` region (e.g., `Crusher` / `OreWasher`)
2. **Recipe Table**: Define `Recipe` struct and `recipes` vector inside machine class
3. **UI Panel**: Implement corresponding `openXxxUI()` method (refer to `openCrusherUI`)
4. **Build Integration**: Add option in `openBuildUI()` `placeEntries`, implement `placeXxx()` method
5. **Save Integration**: Add machine metadata in `saveGame()` and `loadGame()`
6. **Grid Integration**: Add new machine characters in `tickPowerGrid()` conductivity check
7. **E Key Interaction**: Add `findNearbyMachine('X', ...)` detection in `processKey` `'E'` branch

---

### Commit Guidelines

#### Commit Message Format

```
<type>(<scope>): <subject>

[body]

[footer]
```

**Type (required)**

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation update (README / wiki) |
| `style` | Code formatting (indentation, spacing) |
| `refactor` | Refactoring (no functional change) |
| `test` | Testing related |
| `chore` | Build / toolchain changes |

**Scope (optional)**: Module name, e.g., `furnace`, `lathe`, `power`, `build`, `save`

**Subject**: Short description, max 50 chars, imperative mood

**Body (optional)**: Detailed explanation, max 72 chars per line

**Footer (optional)**: Issue reference, e.g., `Closes #42`

#### Examples

```
feat(crusher): add 4-slot ore crusher machine

- Implement Crusher class with 4 parallel slots
- Add openCrusherUI() FTXUI panel
- Integrate with build mode and power grid
- Support 8 ore types -> crushed ore + gravel

Closes #17
```

```
fix(lathe): prevent crash when loading saved game without lathe meta

Lathe position was hardcoded to (8,5), causing out-of-bounds access
when game is loaded from a save without lathe machineMeta entry.

Now dynamically find lathe from machineMeta on each tick.
```

---

### Testing Requirements

#### Build Testing

- **MSBuild must return 0** (no compile errors/warnings)
- Warning level `/W3`, new code should not introduce new warnings

#### Functional Testing (Manual)

Before submitting a PR, please verify at least these scenarios:

| Module | Test Items |
|--------|------------|
| Startup | Enter save selection → new game/load save works |
| Movement | WASD / arrow keys move, boundary collision works |
| Mining | E near mineral, 3 hits to get ore |
| Furnace | F hint, E opens panel, load/smelt/collect works |
| Lathe | E opens panel, select mold/load/process (with/without power) |
| Power Grid | Feed coal → lay wires → lathe powers on; power cut auto-pauses |
| Build | B opens panel, place generator/wire/decor, validation works |
| Trade | T opens, SELL/BUY toggle, buy/sell/blueprint unlock works |
| Bank | K opens, asset prices update, buy/sell works |
| Quests | J opens, quest progress tracking works |
| Save | P save → quit → reload, progress fully restored |
| Travel | Car → Wasteland → Cave → Home, map switching works |

#### Regression Testing

If modifying core modules (save, power grid, machine progress), test old save compatibility:

1. Generate a save with old version (`chemical_world_slot1.sav`)
2. Load with your modified version
3. Check critical data (player position, items, machine state) is fully restored

---

### FAQ

#### Q1: Can't find FTXUI headers during compilation

**A**: Check include paths in `.vcxproj`, confirm they point to FTXUI's `include/` directory. Example:
```xml
<AdditionalIncludeDirectories>
    D:\360Downloads\ftxui-6.1.9-win64\include;
    %(AdditionalIncludeDirectories)
</AdditionalIncludeDirectories>
```

#### Q2: FTXUI UI shows garbled/blank at runtime

**A**: Ensure terminal supports UTF-8 and ANSI colors. Run in Windows Terminal or VS Code terminal; legacy CMD is not recommended.

#### Q3: Enter key doesn't work in FTXUI UI (buttons invisible)

**A**: Check if buttons are hidden by `Maybe` guard conditions, or not added to `Container` focus chain. Common mistake:
```cpp
// ❌ Wrong: buttons is temporary, not assigned to variable
Renderer(Container::Vertical({ buttonA, buttonB }), [&] { ... });

// ✅ Correct
auto buttons = Container::Vertical({ buttonA, buttonB });
Renderer(buttons, [&] { ... });
```

#### Q4: Machine positions lost after loading save (ghost tiles)

**A**: `generate()` resets tiles, but `machineMeta` preserves metadata. Call `rehydrateMachineTiles()` to rebuild tile display from meta. This is automatically called at the end of `loadGame()`.

#### Q5: How to add new machine characters to grid BFS conductivity?

**A**: Add to `isConductive` lambda in `tickPowerGrid()`:
```cpp
if (d == 'X' || d == 'x' || d == 'W' || d == 'w' || d == 'R' || d == 'r') return true;
```

---

Thank you again for your contribution! May your code 🧪 be stable, ⚡ efficient, and 🚀 bug-free forever.

---

*Last updated: 2026-08-30*
