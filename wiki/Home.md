# Chemical-World Wiki

欢迎来到 **Chemical-World** Wiki！这是一个基于 C++20 + FTXUI 构建的终端工业沙盒游戏，灵感来自 GT:New Horizons 的简化电力网络与机器链。

---

## 📚 文档索引

### 入门
- [Gameplay Basics](Gameplay-Basics.md) — 新手玩法、出生位置、地图结构
- [Controls](Controls.md) — 完整按键列表与 FTXUI 操作技巧

### 系统
- [Power System](Power-System.md) — 电力网络、EU pool、发电机、电线连通
- [Machines](Machines.md) — 土高炉、车床、火力发电机详解
- [Build Mode](Build-Mode.md) — B 键建造模式、7×7 放大网格、蓝图解锁
- [Trading Market](Trading-Market.md) — SELL/BUY 双标签、定价规则、蓝图
- [Save System](Save-System.md) — v3 存档格式、v2 兼容、3 档位 + autosave

---

## 🎯 游戏目标

挖掘矿石 → 冶炼金属 → 车床加工钢零件 → 卖钱或装配更多机器 → 搭建工业帝国。

**典型流程**：
1. 在家园按 `H` 看按键
2. 走到 Car 旁按 `E` 去荒原挖矿
3. 回家按 `E` 走到土高炉旁，烧炼 4 槽矿石（30s/槽）
4. 按 `E` 走到车床旁，选模具加工钢零件（需要电力！）
5. 按 `T` 卖东西赚钱，按 `B` 进入建造模式买蓝图、放发电机
6. 按 `P` 保存进度

---

## 🔧 技术信息

- **源码**：单文件 `Chemical-World.cpp`（约 3100 行）
- **编译**：MSVC / C++20 / x64
- **UI 库**：FTXUI 6.1.9
- **平台**：Windows（控制台 ANSI）
- **存档**：v3 二进制文本混合格式

详见 [README](../README.md)。
