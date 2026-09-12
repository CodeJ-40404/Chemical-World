# Chemical-World Wiki

**EN:** Welcome to the official wiki of **Chemical-World** — a terminal industrial-chemistry sandbox built with C++20 and FTXUI, inspired by the simplified power network and machine chains of GT:New Horizons.

**中文：** 欢迎来到 **Chemical-World** 官方 Wiki！这是一个基于 C++20 + FTXUI 构建的终端工业化学沙盒游戏，灵感来自 GT:New Horizons 的简化电力网络与机器链。

> **Current version / 当前版本：** v0.5.0 *Electrolysis Update* · save format **v7** · source single-file ~6,300 lines · release notes: [RELEASES.md](../RELEASES.md)

---

## 📚 Index / 文档索引

### Getting started / 入门

| Page / 页面 | EN | 中文 |
|-------------|----|------|
| [Gameplay Basics](Gameplay-Basics.md) | Spawn, maps, mining, crafting, XP | 出生、地图、挖矿、合成、经验 |
| [Controls](Controls.md) | Complete key bindings & FTXUI tips | 完整按键列表与 FTXUI 技巧 |

### Systems / 系统

| Page / 页面 | EN | 中文 |
|-------------|----|------|
| [Power System](Power-System.md) | EU pool, generators, wire BFS, powerDraw | EU 池、发电机、电线 BFS、powerDraw |
| [Machines](Machines.md) | Furnace, Lathe, Generator, processing chain, Electrolyzer, Chemistry Bench | 高炉、车床、发电机、处理链、电解机、化合台 |
| [Chemistry](Chemistry.md) ⭐NEW | 15 elements, compounds, 9 recipes, economy | 15 元素、化合物、9 配方、经济 |
| [Build Mode](Build-Mode.md) | B key, 7×7 grid, 10 placeables, blueprints | B 键、7×7 网格、10 种可放置物、蓝图 |
| [Trading Market](Trading-Market.md) | SELL/BUY, pricing rules, 8 blueprints | SELL/BUY、定价规则、8 种蓝图 |
| [Save System](Save-System.md) | v7 format, v2–v7 compatibility, slots | v7 格式、v2–v7 兼容、档位 |

### Community / 社区

| Page / 页面 | EN | 中文 |
|-------------|----|------|
| [Proposal Standard](Proposal-Standard.md) | How to spec a feature/balance change | 如何撰写功能/平衡提案 |
| [Join Us](Join-Us.md) | Roles, good first issues, workflow | 角色、新手任务、贡献流程 |

---

## 🎯 Goal / 游戏目标

**EN:** Mine ores → process & smelt metals → machine steel parts → **electrolyze compounds into elements → synthesize high-value chemicals** → earn coins and build ever more machines.

**中文：** 挖矿 → 处理并冶炼金属 → 车床加工钢零件 → **电解化合物得到元素 → 合成高价值化学品** → 赚钱并建造更多机器。

**Typical flow / 典型流程：**

1. **EN:** Press `H` at Home for the key list. **中文：** 在家园按 `H` 查看按键。
2. **EN:** Walk to the Car, press `E` to mine in the Wasteland. **中文：** 走到 Car 旁按 `E` 去荒原挖矿。
3. **EN:** Smelt 4 ore slots in the blast furnace (30s each, no power). **中文：** 土高炉烧炼 4 槽矿石（每槽 30s，无需电）。
4. **EN:** Machine steel parts on the Lathe (needs power!). **中文：** 车床加工钢零件（需要电力！）。
5. **EN:** `T` to trade; `B` to buy blueprints and place generators/machines. **中文：** `T` 交易；`B` 买蓝图并放置发电机/机器。
6. **EN (v0.5.0+):** Buy salt/water → Electrolyzer (`Z`) → elements → Chemistry Bench (`K`) → chemicals. **中文（v0.5.0+）：** 买盐/水 → 电解机（`Z`）→ 元素 → 化合台（`K`）→ 化学品。
7. **EN:** Press `P` to save. **中文：** 按 `P` 保存进度。

---

## 🔧 Technical facts / 技术信息

| EN | 中文 |
|----|------|
| Source: single file `Chemical-World.cpp` (~6,300 lines) | 源码：单文件 `Chemical-World.cpp`（约 6,300 行） |
| Compiler: MSVC 2022 / C++20 / x64 | 编译：MSVC 2022 / C++20 / x64 |
| UI: FTXUI 6.1.9 (fullscreen + 100ms ticker) | UI：FTXUI 6.1.9（全屏 + 100ms ticker） |
| Platform: Windows 10/11 (ANSI console) | 平台：Windows 10/11（ANSI 控制台） |
| Save: v7 binary/text hybrid, v2+ compatible | 存档：v7 二进制文本混合，兼容 v2+ |

See [README](../README.md) for build instructions. / 构建方法见 [README](../README.md)。
