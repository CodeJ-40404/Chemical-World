# Chemical-World

[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![FTXUI](https://img.shields.io/badge/FTXUI-6.1.9-green.svg)](https://github.com/ArthurSonzogni/FTXUI)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)

一个基于 C++ 和 FTXUI 构建的终端交互式沙盒游戏，模拟化学工业与资源管理流程。

---

## 📁 项目结构

```
Chemical-World/
├── Chemical-World.cpp          # 主程序入口与游戏核心逻辑
├── README.md                   # 项目说明文档
└── .vs/                        # Visual Studio 本地缓存（已忽略）
    └── ...
```

### 核心模块说明

| 模块 | 文件位置 | 功能描述 |
|------|----------|----------|
| **游戏主循环** | `Chemical-World.cpp` / `ChemicalWorldGame::run()` | 状态机驱动，处理输入与渲染 |
| **地图系统** | `GameMap` 类 | 30×20 网格地图，含矿物生成与碰撞检测 |
| **高炉系统** | `BlastFurnace` 类 | 温度模拟、配方匹配、进度追踪 |
| **FTXUI 界面** | `FurnaceUI` 类 | 高炉操作面板，多线程事件驱动 |
| **玩家数据** | `PlayerData` 结构体 | 背包、经验、等级、位置管理 |
| **合成系统** | `craft()` 函数 | 基于配方的物品合成逻辑 |
| **交易系统** | `trade()` 函数 | 动态定价与物品交易机制 |

---

## 🛠️ 构建与依赖

### 环境要求
- **编译器**：MSVC (Windows)
- **C++ 标准**：C++17/20，建议C++20
- **终端**：Windows 控制台（需支持 ANSI 颜色）

### 依赖库
| 库名称 | 版本 | 用途 |
|--------|------|------|
| [FTXUI](https://github.com/ArthurSonzogni/FTXUI) | 6.1.9 | 高炉界面的终端 UI 渲染 |
| Windows API | - | 控制台颜色控制 (`SetConsoleTextAttribute`) |

### 构建步骤（Visual Studio）
1. 克隆仓库：
   ```bash
   git clone https://github.com/CodeJ-40404/Chemical-World.git
   cd Chemical-World
   ```
2. 使用 vcpkg 安装 FTXUI：
   ```bash
   vcpkg install ftxui
   ```
3. 在 VS 中打开项目，配置 vcpkg 集成（`vcpkg integrate install`）。
4. 选择 `x64` 平台，生成解决方案。

---

## 🧪 贡献指南

### 代码规范
- **命名**：
  - 类名：`PascalCase`（如 `BlastFurnace`）
  - 函数名：`camelCase`（如 `addItem()`）
  - 私有成员：`camelCase` 或 `m_` 前缀（可选）
- **注释**：中文注释允许，但游戏内显示文本请使用**英文 + ASCII 字符**（避免终端乱码）。
- **头文件顺序**：FTXUI 头 → Windows API → STL → 项目自定义头。

### 提交指南
1. Fork 本项目。
2. 创建功能分支：`git checkout -b feature/your-awesome-features`
3. 提交变更：`git commit -m "feat: 增加了xxx"`
4. 推送分支：`git push origin feature/your-feature`
5. 发起 Pull Request，描述变更内容与测试结果。

### 测试要求
- 至少保证代码可编译通过
- 游戏能正常游玩/操作，不会莫名闪退
- 不过于偏离项目方向
- 没有严重的环境错误

---

## 🚀 项目发展路线

### 已完成
- [x] 基础地图渲染与移动
- [x] 矿物生成与开采系统（3 次敲击机制）
- [x] 高炉温控与配方系统
- [x] FTXUI 高炉操作面板
- [x] 合成与交易系统
- [x] 经验值与等级系统

### 规划中（Roadmap）
- [ ] **存档系统**：JSON/二进制序列化，支持断点续玩
- [ ] **多配方扩展**：动态配方注册机制（无需修改核心代码）
- [ ] **AI 商人**：基于价格波动的 NPC 交易逻辑
- [ ] **跨平台适配**：Linux/macOS 终端支持（通过 `ncurses` 替换 Windows API）
- [ ] **单元测试**：使用 Google Test 覆盖核心逻辑
- [ ] **性能优化**：减少 `_getch()` 阻塞，改为事件驱动轮询

### 长期愿景
成为一个庞大且融合工业-化学-宇航的硬核游戏，再基础框架完工后，保证没有魔法合成

---

## 📜 许可证

[MIT License](https://github.com/CodeJ-40404/Chemical-World?tab=MIT-1-ov-file) © 2026 [CodeJ-40404]


---

## 📌 关于游戏教程

**本文档不包含游戏玩法教程或操作指南。**  
如需了解具体的游戏内容、操作方式或新手引导，请查阅：
- 游戏内帮助面板（按 `H` 键）
- 项目 Wiki（待建设）
- 或联系维护团队获取玩家手册

---

## 🤝 联系方式

- 问题反馈：GitHub Issues
- 讨论交流：Discord 群组（[暂时没有]）
- 维护者：[@CodeJ-40404](https://github.com/CodeJ-40404)

---

*Happy Gaming!* 🧪⚗️
