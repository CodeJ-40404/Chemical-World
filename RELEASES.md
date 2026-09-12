# Release Notes / 版本发布说明

[English](#english) | [中文](#中文)

---

<a id="english"></a>
# v1.3.2 — Electrolysis Update

**Release date:** 2026-09-12
**Save format:** v7 (loads v2–v7)
**Source:** single-file `Chemical-World.cpp` (~6,300 lines)
**Build:** MSVC 2022 / C++20 / x64 — `MSBuild EXIT CODE = 0`, single UTF-8 BOM
**Spec / review:** `.trae/specs/electrolysis-update/` (spec.md · tasks.md · review.md)

## Summary

This release adds the first **real-chemistry processing layer** to Chemical-World. Two new endgame machines — the **Electrolyzer** and the **Chemistry Bench** — close a reversible loop between compounds and elements:

```
ore chain ─► compounds ──[Electrolyzer 10 EU/t]──► elements ──[Chemistry Bench]──► high-value compounds
```

15 elements (9 new elemental items + 6 existing metal ingots treated as elements), 4 new compounds, 9 grounded recipes inspired by real electrolysis/synthesis reactions, and two new inventory categories ship in this update. An independent review pass (11 checkpoints) was run before release; all three findings it raised were fixed and re-verified.

## Highlights

- ⚡ **Electrolyzer (`Z`)** — a powered deep-processing terminal on par with the Centrifuge: 10 EU/tick, 8 s per run, multi-product output, auto-pauses without power.
- 🧪 **Chemistry Bench (`K`)** — a hand-powered synthesis station (**no EU cost**) that recombines elements into compounds, making the chemistry loop reversible.
- 🧬 **15 elements** grounded in the periodic table: H, O, C, N, S, Cl, Na, Mg, Si plus Fe/Cu/Al/Sn/Ag/Au (reusing existing ingots — no duplicate stacks).
- 🔁 **9 chemistry recipes** modeled on textbook reactions: water electrolysis, alumina electrolysis, molten-salt electrolysis, and the chlor-alkali process; plus 5 bench syntheses including NaOH, CuSO₄ and FeS.
- 💰 **New money routes** — electrolysis chains convert cheap buyable feedstock (salt 45c, water 10c) into elements/compounds worth far more (best chain: chlor-alkali, up to ~200c output from ~55c feedstock).
- 🎒 **Three new backpack tabs**: BASIC / RAW, ELEMENTS, COMPOUNDS.
- 💾 **Save format v7**, fully backward compatible with v2–v6.

## New Content

### Elements (15)

Nine new elemental items, priced and sellable:

| Element | Item name | Sell price | Obtainable in v0.5.0 |
|---------|-----------|-----------|----------------------|
| Hydrogen | `hydrogen` | 30c | Electrolyzer (water / chlor-alkali) |
| Oxygen | `oxygen` | 25c | Electrolyzer (water / alumina) |
| Carbon | `carbon` | 20c | Reserved — no source yet (see Known Limitations) |
| Nitrogen | `nitrogen` | 35c | Reserved — no source yet |
| Sulfur | `sulfur` | 40c | Reserved — no source yet |
| Chlorine | `chlorine` | 50c | Electrolyzer (salt / chlor-alkali) |
| Sodium | `sodium` | 60c | Electrolyzer (salt) |
| Magnesium | `magnesium` | 45c | Reserved — no source yet |
| Silicon | `silicon` | 55c | Reserved — no source yet |

Six existing metal ingots double as elements and intentionally share the furnace stack (category `product`, no duplicate items):

| Element | Ingot item | Sell price |
|---------|-----------|-----------|
| Iron (Fe) | `iron_ingot` | 16c |
| Copper (Cu) | `copper` | 48c |
| Aluminum (Al) | `aluminum` | 26c |
| Tin (Sn) | `tin` | 95c |
| Silver (Ag) | `silver_ingot` | 600c |
| Gold (Au) | `gold_ingot` | 1500c |

### Compounds (4 new + 2 reused)

| Compound | Item name | Sell price | Notes |
|----------|-----------|-----------|-------|
| Salt | `salt` | 15c | **Buyable** (BUY tab, 45c) — electrolysis entry point |
| Sodium hydroxide | `sodium_hydroxide` | 120c | Chlor-alkali product / bench craftable |
| Copper(II) sulfate | `copper_sulfate` | 220c | Bench-only high-value product |
| Iron(II) sulfide | `iron_sulfide` | 90c | Bench-only sellable product |
| Water | `water` | 2c | Reused; buyable (10c) |
| Alumina (bauxite dust) | `bauxite_dust` | 22c | Reused from the processing chain; not buyable |

### New machine: Electrolyzer (map char `Z` / `z`)

| Property | Value |
|----------|-------|
| Footprint | 2×2, anchor `Z` (bright cyan) + helpers `z` (dark cyan), impassable |
| Blueprint | **300c** (Trade ▸ BUY, one-time) |
| Placement cost | **250c** (Build menu, index 6) |
| Power draw | **10 EU / tick** while processing — the hungriest machine in the game |
| Cycle time | 8,000 ms (80 ticks) |
| States | Idle → Electrolyzing → Done; pauses with progress preserved when unpowered |
| UI | Recipe radiobox · LOAD / COLLECT / CLOSE · live progress bar & 100 ms ticker |

**Recipes:**

| # | Recipe | Input | Output |
|---|--------|-------|--------|
| 1 | Electrolyze Water | water ×1 | hydrogen ×2, oxygen ×1 |
| 2 | Electrolyze Alumina | bauxite_dust ×1 | aluminum ×2, oxygen ×3 |
| 3 | Electrolyze Salt | salt ×1 | sodium ×1, chlorine ×1 |
| 4 | Chlor-alkali Process | salt ×1 + water ×1 | chlorine ×1, hydrogen ×1, sodium_hydroxide ×1 |

### New machine: Chemistry Bench (map char `K` / `k`)

| Property | Value |
|----------|-------|
| Footprint | 2×2, anchor `K` (bright green) + helpers `k` (dark green), impassable |
| Blueprint | **180c** (Trade ▸ BUY, one-time) |
| Placement cost | **120c** (Build menu, index 7) |
| Power draw | **None** — instant manual crafting |
| UI | Recipe menu with live have/need indicators (green/red) · CRAFT / CLOSE |

**Recipes:**

| # | Recipe | Input | Output |
|---|--------|-------|--------|
| 1 | Synthesize Water | hydrogen ×2 + oxygen ×1 | water ×1 |
| 2 | Synthesize Salt | sodium ×1 + chlorine ×1 | salt ×1 |
| 3 | Synthesize Sodium Hydroxide | sodium ×1 + oxygen ×1 + hydrogen ×1 | sodium_hydroxide ×1 |
| 4 | Synthesize Copper Sulfate | copper ×1 + sulfur ×1 + oxygen ×4 | copper_sulfate ×1 |
| 5 | Synthesize Iron Sulfide | iron_ingot ×1 + sulfur ×1 | iron_sulfide ×1 |

### Economy at a glance (sell-value basis)

| Chain | Feedstock cost (BUY) | Output sell value | Gross margin |
|-------|---------------------|-------------------|--------------|
| Water electrolysis | 10c | 85c | **+75c** / 8 s + 800 EU |
| Molten-salt electrolysis | 45c | 110c | **+65c** / 8 s + 800 EU |
| Chlor-alkali | 55c (salt + water) | 200c | **+145c** / 8 s + 800 EU |
| Alumina electrolysis | 22c (dust, self-produced) | 127c | **+105c** / 8 s + 800 EU |
| Bench: NaOH synthesis | 115c in elements | 120c | +5c |
| Bench: CuSO₄ synthesis | 188c in elements | 220c | **+32c** |
| Bench: FeS synthesis | 56c in elements | 90c | **+34c** |

> Reverse syntheses (water, salt) are intentionally value-negative: they exist for material reconversion, not profit.

## Integration Details

- **Build menu** now lists 10 entries (indices 0–9): Generator, Wire, Crusher, Washer, Centrifuge, Gem Sorter, **Electrolyzer**, **Chemistry Bench**, Flower, Grass tuft.
- **E-key dispatch** opens `openElectrolyzerUI` near `Z` and `openChemBenchUI` near `K`.
- **Home legend** shows `Z = Electrolyzer`, `K = Chemistry Bench`; build preview colors match.
- **Backpack** gains `BASIC / RAW` (blue), `ELEMENTS` (cyan), `COMPOUNDS` (green) tabs; metal outputs from the electrolyzer merge into the existing `product` stacks.
- **Trade ▸ BUY** adds salt, water, Electrolyzer Blueprint (300c) and Chemistry Bench Blueprint (180c). Raw **elements are deliberately not sold** — electrolysis is their exclusive source.
- **Global tick:** the Electrolyzer participates in the 100 ms tick (draws 10 EU while processing); the Chemistry Bench does not tick.
- **Map character audit:** `Z`/`K` collide with neither ore chars (`H M B T P U G S` + variants) nor existing machine chars (`G X W R F L S`).

## Save Migration (v6 → v7)

- Header is now `CHEMICAL_WORLD_SAVE 7`; the blueprint line stores **8 ints** (generator, wire, crusher, washer, centrifuge, sorter, **electrolyzer**, **chembench**).
- `loadGame` accepts v2–v7. On v≤6 the two new blueprint flags default to `false` and the trailing ints are not read — your old saves load untouched.
- Machine instances remain unserialized by design: they are reconstructed from `machineMeta` via find-or-insert and tile rehydration, same as all other machines.

## Fixes in This Release

These were caught by the pre-release independent review and fixed before tagging:

1. **[P0] Electrolyzer could never receive power.** The power-grid BFS (`isConductive`) and the `poweredMachines` registration loop did not include the new `Z` character, so every `powerDraw(10, …)` returned false and the machine sat at IDLE forever.
2. **[Pre-existing] Gem Sorter (`S`) had the same grid-registration bug** — it was never added to the conductor set. Fixed in the same change; sorters built on old saves now power correctly.
3. **[P2] Water was invisible in the backpack.** It used category `basic`, for which no tab existed. Added the `BASIC / RAW` tab (also covers starting water).
4. **[P2] Copper sulfate synthesis lost money** (188c inputs → 180c output). Sell price raised 180c → **220c** (+32c margin).

## Known Limitations

- `sulfur`, `carbon`, `nitrogen`, `magnesium` and `silicon` exist as priced items and appear in future-facing recipe scaffolding, but **have no in-game source yet**. Consequently the CuSO₄ and FeS bench recipes cannot be completed in survival until sulfur production lands (planned: sulfide ores / Frasch-style extraction).
- No balancing for EU cost vs. throughput beyond manual review; expect tuning in future patches.
- Machines still cannot be dismantled after placement.
- Windows-only; the Linux/macOS port remains on the roadmap.

## Upgrade Instructions

1. Pull/replace the source and rebuild with MSBuild (Debug ▸ x64) — see README §Build.
2. Launch with your existing save: v2–v6 saves load directly. Press `P` once to re-save as v7.
3. In-game: `T` ▸ BUY ▸ purchase **Electrolyzer Blueprint (300c)** and **Chemistry Bench Blueprint (180c)** → `B` to place → run wires from a powered generator → `E` to operate.

## Verification

- MSBuild Debug x64: **EXIT CODE = 0**; `Chemical-World.exe` produced.
- Single UTF-8 BOM confirmed; no new compiler warnings beyond the pre-existing FTXUI LNK4099 PDB notice.
- Independent review: 11 checkpoints (10 rule + 1 rubric), R1 `fail` (3 findings) → all fixed → R2 `pass`.
- Acceptance coverage AC-1 … AC-9 — all satisfied; evidence in `.trae/specs/electrolysis-update/review.md`.

---

<a id="中文"></a>
# v1.3.2 — 电解更新（Electrolysis Update）

**发布日期：** 2026-09-12
**存档格式：** v7（可加载 v2–v7）
**源码：** 单文件 `Chemical-World.cpp`（约 6,300 行）
**构建：** MSVC 2022 / C++20 / x64 —— `MSBuild EXIT CODE = 0`，单 UTF-8 BOM
**规格/审查：** `.trae/specs/electrolysis-update/`（spec.md · tasks.md · review.md）

## 更新概述

本版本为 Chemical-World 加入第一层**有真实化学依据的深加工系统**。两台新的后期机器——**电解机**与**化合台**——构成化合物与元素之间的可逆闭环：

```
矿物处理链 ─► 化合物 ──[电解机 10 EU/t]──► 元素 ──[化合台]──► 高价值化合物
```

本次更新包含 15 种元素（9 种新元素物品 + 6 种现有金属锭作元素）、4 种新化合物、9 条源自真实电解/化合反应的配方，以及两个新背包分类。发布前进行了一轮独立审查（11 个检查点），发现的 3 个问题已全部修复并复验通过。

## 核心亮点

- ⚡ **电解机（`Z`）**：与离心机并列的耗电型深加工终端，10 EU/tick、8 秒/次、多产物、断电自动暂停。
- 🧪 **化合台（`K`）**：手动合成台（**不耗电**），将元素重新组合为化合物，化学链可逆。
- 🧬 **15 种元素**：H、O、C、N、S、Cl、Na、Mg、Si，外加 Fe/Cu/Al/Sn/Ag/Au（复用现有锭，不产生重名栈）。
- 🔁 **9 条化学配方**：水电解、氧化铝电解、熔盐电解、氯碱工艺；以及 NaOH、CuSO₄、FeS 等 5 条台成配方。
- 💰 **新赚钱路线**：廉价可买原料（盐 45c、水 10c）电解为高价值元素/化合物（最优氯碱链：约 55c 原料 → 200c 产出）。
- 🎒 **三个新背包页**：BASIC / RAW、ELEMENTS、COMPOUNDS。
- 💾 **存档 v7**，对 v2–v6 完全向后兼容。

## 新增内容

### 元素（15 种）

9 种新元素物品，均明码标价、可出售：

| 元素 | 物品名 | 售价 | v0.5.0 获取途径 |
|------|--------|------|----------------|
| 氢 H | `hydrogen` | 30c | 电解机（水 / 氯碱） |
| 氧 O | `oxygen` | 25c | 电解机（水 / 铝土） |
| 碳 C | `carbon` | 20c | 预留，暂无来源（见已知限制） |
| 氮 N | `nitrogen` | 35c | 预留，暂无来源 |
| 硫 S | `sulfur` | 40c | 预留，暂无来源 |
| 氯 Cl | `chlorine` | 50c | 电解机（盐 / 氯碱） |
| 钠 Na | `sodium` | 60c | 电解机（盐） |
| 镁 Mg | `magnesium` | 45c | 预留，暂无来源 |
| 硅 Si | `silicon` | 55c | 预留，暂无来源 |

6 种现有金属锭同时作为元素，刻意与炉子产物共用同一堆叠（分类 `product`，不新增重名物品）：

| 元素 | 锭物品 | 售价 |
|------|--------|------|
| 铁 Fe | `iron_ingot` | 16c |
| 铜 Cu | `copper` | 48c |
| 铝 Al | `aluminum` | 26c |
| 锡 Sn | `tin` | 95c |
| 银 Ag | `silver_ingot` | 600c |
| 金 Au | `gold_ingot` | 1500c |

### 化合物（4 新 + 2 复用）

| 化合物 | 物品名 | 售价 | 说明 |
|--------|--------|------|------|
| 盐 | `salt` | 15c | **可买入**（BUY 页 45c）——电解入口原料 |
| 氢氧化钠 | `sodium_hydroxide` | 120c | 氯碱产物 / 可化合 |
| 硫酸铜 | `copper_sulfate` | 220c | 仅化合产出的高价值品 |
| 硫化亚铁 | `iron_sulfide` | 90c | 仅化合产出，可卖 |
| 水 | `water` | 2c | 复用；可买入（10c） |
| 氧化铝（铝土粉） | `bauxite_dust` | 22c | 复用处理链产物；不可买入 |

### 新机器：电解机（地图字符 `Z` / `z`）

| 属性 | 数值 |
|------|------|
| 占地 | 2×2，锚点 `Z`（亮青）+ 辅助格 `z`（暗青），不可通行 |
| 蓝图 | **300c**（T ▸ BUY，一次性） |
| 放置费 | **250c**（建造菜单第 6 项） |
| 耗电 | 加工中 **10 EU/tick**——全游戏最高 |
| 周期 | 8,000 ms（80 tick） |
| 状态 | Idle → Electrolyzing → Done；断电暂停且保留进度 |
| 界面 | 配方 Radiobox · LOAD / COLLECT / CLOSE · 实时进度条 + 100ms ticker |

**配方：**

| # | 配方 | 输入 | 输出 |
|---|------|------|------|
| 1 | Electrolyze Water（水电解） | water ×1 | hydrogen ×2, oxygen ×1 |
| 2 | Electrolyze Alumina（铝土电解） | bauxite_dust ×1 | aluminum ×2, oxygen ×3 |
| 3 | Electrolyze Salt（熔盐电解） | salt ×1 | sodium ×1, chlorine ×1 |
| 4 | Chlor-alkali（氯碱工艺） | salt ×1 + water ×1 | chlorine ×1, hydrogen ×1, sodium_hydroxide ×1 |

### 新机器：化合台（地图字符 `K` / `k`）

| 属性 | 数值 |
|------|------|
| 占地 | 2×2，锚点 `K`（亮绿）+ 辅助格 `k`（暗绿），不可通行 |
| 蓝图 | **180c**（T ▸ BUY，一次性） |
| 放置费 | **120c**（建造菜单第 7 项） |
| 耗电 | **无**——即时手动合成 |
| 界面 | 配方菜单 + 实时 have/need 红绿指示 · CRAFT / CLOSE |

**配方：**

| # | 配方 | 输入 | 输出 |
|---|------|------|------|
| 1 | Synthesize Water（合成水） | hydrogen ×2 + oxygen ×1 | water ×1 |
| 2 | Synthesize Salt（合成盐） | sodium ×1 + chlorine ×1 | salt ×1 |
| 3 | Synthesize Sodium Hydroxide | sodium ×1 + oxygen ×1 + hydrogen ×1 | sodium_hydroxide ×1 |
| 4 | Synthesize Copper Sulfate（硫酸铜） | copper ×1 + sulfur ×1 + oxygen ×4 | copper_sulfate ×1 |
| 5 | Synthesize Iron Sulfide（硫化亚铁） | iron_ingot ×1 + sulfur ×1 | iron_sulfide ×1 |

### 经济收益速览（按售价计）

| 链路 | 原料成本（BUY） | 产物售价 | 毛利 |
|------|----------------|----------|------|
| 水电解 | 10c | 85c | **+75c** / 8 秒 + 800 EU |
| 熔盐电解 | 45c | 110c | **+65c** / 8 秒 + 800 EU |
| 氯碱 | 55c（盐+水） | 200c | **+145c** / 8 秒 + 800 EU |
| 铝土电解 | 22c（自产矿粉） | 127c | **+105c** / 8 秒 + 800 EU |
| 化合 NaOH | 元素 115c | 120c | +5c |
| 化合 CuSO₄ | 元素 188c | 220c | **+32c** |
| 化合 FeS | 元素 56c | 90c | **+34c** |

> 逆向合成（水、盐）刻意做成亏损：用途是物质转换，而非赚钱。

## 集成细节

- **建造菜单**现有 10 项（序号 0–9）：发电机、电线、破碎机、洗矿槽、离心机、宝石筛选机、**电解机**、**化合台**、花、草丛。
- **E 键分发**：靠近 `Z` 打开电解机 UI，靠近 `K` 打开化合台 UI。
- **家园图例**新增 `Z = Electrolyzer`、`K = Chemistry Bench`；建造预览配色同步。
- **背包**新增 `BASIC / RAW`（蓝）、`ELEMENTS`（青）、`COMPOUNDS`（绿）三个分类页；电解产出的金属锭并入既有 `product` 堆叠。
- **T ▸ BUY** 新增 salt、water、电解机蓝图（300c）、化合台蓝图（180c）。**元素刻意不售卖**——电解是唯一来源。
- **全局 tick**：电解机接入 100ms 心跳（加工时扣 10 EU）；化合台不进 tick。
- **字符冲突审计**：`Z`/`K` 与矿物字符（`H M B T P U G S` 等）及机器字符（`G X W R F L S`）均不冲突。

## 存档迁移（v6 → v7）

- 文件头改为 `CHEMICAL_WORLD_SAVE 7`；蓝图行写入 **8 个 int**（发电机、电线、破碎、洗矿、离心、筛选、**电解机**、**化合台**）。
- `loadGame` 接受 v2–v7。v≤6 时两个新蓝图标记默认 `false`，且不读取末尾两个 int——旧档原样加载。
- 机器实例按既有设计不序列化：通过 `machineMeta` find-or-insert 与瓦片 rehydrate 重建，与其他机器一致。

## 本版本修复

均由发布前独立审查发现，在打 tag 前修复：

1. **[P0] 电解机永远无法通电。** 电网 BFS（`isConductive`）与 `poweredMachines` 注册循环未包含新字符 `Z`，导致 `powerDraw(10, …)` 恒为 false，机器永远停在 IDLE。
2. **[遗留问题] 宝石筛选机 `S` 存在同样的电网注册缺陷**——从未加入导体集。本次一并修复；旧档中已建的筛选机现在可以正常通电。
3. **[P2] 水在背包中不可见。** 其分类为 `basic` 但背包无对应页。新增 `BASIC / RAW` 页（同时覆盖初始赠水）。
4. **[P2] 硫酸铜合成亏本**（188c 投入 → 180c 产出）。售价 180c 上调至 **220c**（毛利 32c）。

## 已知限制

- `sulfur`、`carbon`、`nitrogen`、`magnesium`、`silicon` 已有定价并为后续配方预留，但**当前版本没有获取途径**。因此在硫的来源上线前（规划：硫化矿 / 类似 Frasch 采硫），CuSO₄ 与 FeS 配方在生存模式下无法完成。
- EU 成本与吞吐量平衡仅经过人工审查，后续版本可能调整数值。
- 机器放置后仍不可拆除。
- 仅支持 Windows；Linux/macOS 移植仍在路线图上。

## 升级步骤

1. 拉取/替换源码并用 MSBuild 重新生成（Debug ▸ x64），见 README §构建。
2. 直接用旧存档启动：v2–v6 均可加载。按一次 `P` 即可另存为 v7。
3. 游戏内：`T` ▸ BUY ▸ 购买**电解机蓝图（300c）**与**化合台蓝图（180c）** → `B` 放置 → 从通电的发电机接线 → `E` 操作。

## 验证情况

- MSBuild Debug x64：**EXIT CODE = 0**，成功生成 `Chemical-World.exe`。
- 确认单 UTF-8 BOM；除既有的 FTXUI LNK4099 PDB 提示外无新增警告。
- 独立审查：11 个检查点（10 规则 + 1 评分），R1 `fail`（3 项发现）→ 全部修复 → R2 `pass`。
- 验收标准 AC-1 … AC-9 全部满足，证据见 `.trae/specs/electrolysis-update/review.md`。

---

## Archive / 历史版本

| Version | Date | Theme | Save |
|---------|------|-------|------|
| v0.5.0 | 2026-09-12 | Electrolysis Update（电解：元素 + 电解机 + 化合台） | v7 |
| v0.4.0 | 2026-08 | Power grid, buyable machines & decor（电网/机器购买/装饰；破碎机·洗矿槽·离心机·宝石筛选机） | v6 |
| v0.3.0 | 2026-08 | Blast furnace rewrite & lathe（土高炉重写 + 车床） | v5 |
| v0.1–v0.2 | — | Baseline: three maps, mining, quests, bank, save v2–v4（三地图/挖矿/任务/银行） | v2–v4 |

> Version numbers pre-v1.3.2 are reconstructed from save-format history for reference; only v1.3.2 is formally tagged.
> v1.3.2 之前的版本号依据存档格式沿革回溯标注，仅 v1.3.2 为正式打 tag 版本。
