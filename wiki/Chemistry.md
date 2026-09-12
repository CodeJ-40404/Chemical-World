# Chemistry System / 化学系统

> **EN:** Electrolysis Update (v1.3.2, save v7). This page documents elements, compounds, the Electrolyzer and the Chemistry Bench, and the reversible chemistry loop.
> **中文：** 电解更新（v1.3.2，存档 v7）。本页介绍元素、化合物、电解机、化合台，以及可逆化学闭环。

**Related / 相关页面：** [Machines](Machines.md) · [Power System](Power-System.md) · [Build Mode](Build-Mode.md) · [Trading Market](Trading-Market.md) · [Save System](Save-System.md)

---

## Overview / 概述

**EN:** Chemistry is a deep-processing layer that sits **after** the ore processing chain. Compounds are broken into elements by the powered Electrolyzer; elements are recombined into high-value compounds at the hand-powered Chemistry Bench.

**中文：** 化学是位于矿物处理链**之后**的深加工层。化合物在耗电的电解机中分解为元素；元素在不耗电的化合台上重新合成为高价值化合物。

```
                     10 EU/t, 8s                         instant, no EU
  compounds ───────────────────────► elements ───────────────────────► compounds
 (water/salt/bauxite_dust)       Electrolyzer  Z      Chemistry Bench  K
                ◄──────────────────────────────────────────────────────
                         bench syntheses (selected reactions)
```

**EN:** Design rules / **中文：设计原则**

| EN | 中文 |
|----|------|
| Integer "shares", no mole balancing or gas/liquid state | 按整数"份"配料，不做摩尔配平，无气液状态 |
| Metal elements reuse existing ingot items (no duplicate stacks) | 金属元素复用现有锭物品（不产生重名堆叠） |
| Raw elements are **never sold** in BUY — electrolysis is the exclusive source | 元素在 BUY 页**永不售卖**——电解是唯一来源 |
| Chemistry is a parallel route; it never replaces smelting | 化学是平行路线，不取代既有冶炼 |

---

## Elements / 元素

**EN:** 15 elements in total: 9 new elemental items + 6 existing metal ingots. All appear in the backpack **ELEMENTS** tab except metal ingots, which stay in **PRODUCT** so they merge with furnace output.

**中文：** 共 15 种元素：9 种新元素物品 + 6 种现有金属锭。除金属锭保留在 **PRODUCT** 分类（与炉子产物合并）外，其余均出现在背包 **ELEMENTS** 标签页。

| Element / 元素 | Item / 物品 | Sell / 售价 | Source / 来源 |
|----------------|-------------|------------|---------------|
| Hydrogen 氢 H | `hydrogen` | 30c | Electrolyzer / 电解机 |
| Oxygen 氧 O | `oxygen` | 25c | Electrolyzer / 电解机 |
| Carbon 碳 C | `carbon` | 20c | ⚠ Reserved, no source yet / 预留，暂无来源 |
| Nitrogen 氮 N | `nitrogen` | 35c | ⚠ Reserved / 预留 |
| Sulfur 硫 S | `sulfur` | 40c | ⚠ Reserved / 预留 |
| Chlorine 氯 Cl | `chlorine` | 50c | Electrolyzer / 电解机 |
| Sodium 钠 Na | `sodium` | 60c | Electrolyzer / 电解机 |
| Magnesium 镁 Mg | `magnesium` | 45c | ⚠ Reserved / 预留 |
| Silicon 硅 Si | `silicon` | 55c | ⚠ Reserved / 预留 |
| Iron 铁 Fe | `iron_ingot` | 16c | Furnace / chain / 土高炉 |
| Copper 铜 Cu | `copper` | 48c | Furnace / 土高炉 |
| Aluminum 铝 Al | `aluminum` | 26c | Furnace **or** Electrolyzer / 高炉**或**电解 |
| Tin 锡 Sn | `tin` | 95c | Furnace / 土高炉 |
| Silver 银 Ag | `silver_ingot` | 600c | Furnace / 土高炉 |
| Gold 金 Au | `gold_ingot` | 1500c | Furnace / 土高炉 |

> **EN / 中文：** C, N, S, Mg, Si are priced and reserved for future recipes (sulfide ores, air separation, etc.). As of v0.5.0 they cannot be obtained in survival, so CuSO₄ / FeS bench recipes are not yet craftable in a normal game.
> C、N、S、Mg、Si 已定价并为后续配方预留（硫化矿、空气分离等）。v0.5.0 中无法在生存模式获取，因此 CuSO₄ / FeS 配方暂无法在正常流程中合成。

---

## Compounds / 化合物

| Compound / 化合物 | Item / 物品 | Sell / 售价 | Buy / 买价 | Notes / 说明 |
|-------------------|-------------|------------|-----------|--------------|
| Water 水 | `water` | 2c | 10c | Starting item 5×; electrolysis feedstock / 初始 5 个；电解原料 |
| Salt 盐 | `salt` | 15c | 45c | **Buyable electrolysis entry** / **可买入的电解入口** |
| Sodium hydroxide 氢氧化钠 | `sodium_hydroxide` | 120c | — | Chlor-alkali product; bench-craftable / 氯碱产物；可化合 |
| Copper sulfate 硫酸铜 | `copper_sulfate` | 220c | — | Bench-only, sell product / 仅化合产出，出售用 |
| Iron sulfide 硫化亚铁 | `iron_sulfide` | 90c | — | Bench-only, sell product / 仅化合产出，出售用 |
| Alumina (bauxite dust) 氧化铝（铝土粉） | `bauxite_dust` | 22c | — | From the ore processing chain / 来自矿物处理链 |

**EN:** Compounds live in the backpack **COMPOUNDS** tab; water lives in **BASIC / RAW**.

**中文：** 化合物在背包 **COMPOUNDS** 标签页；水在 **BASIC / RAW** 标签页。

---

## Electrolyzer / 电解机（`Z`）

| Property / 属性 | Value / 数值 |
|-----------------|--------------|
| Footprint / 占地 | 2×2 — anchor `Z` bright cyan, helpers `z` dark cyan / 锚点亮青，辅助格暗青 |
| Blueprint / 蓝图 | 300c (T ▸ BUY, one-time / 一次性) |
| Placement / 放置费 | 250c (Build menu index 6 / 建造菜单第 6 项) |
| Power / 耗电 | **10 EU / tick** while processing / 加工中（全游戏最高） |
| Cycle / 周期 | 8,000 ms = 80 ticks = **800 EU / run** |
| Unpowered / 断电 | Pauses, progress retained / 暂停并保留进度 |

### Electrolysis recipes / 电解配方

| # | Recipe / 配方 | Input / 输入 | Output / 输出 |
|---|---------------|--------------|---------------|
| 1 | Electrolyze Water / 水电解 | water ×1 | hydrogen ×2 + oxygen ×1 |
| 2 | Electrolyze Alumina (bauxite dust) / 铝土电解 | bauxite_dust ×1 | aluminum ×2 + oxygen ×3 |
| 3 | Electrolyze Salt (molten) / 熔盐电解 | salt ×1 | sodium ×1 + chlorine ×1 |
| 4 | Chlor-alkali (salt + water) / 氯碱工艺 | salt ×1 + water ×1 | chlorine ×1 + hydrogen ×1 + sodium_hydroxide ×1 |

### Operation / 操作

**EN:**
1. Buy the blueprint at `T` ▸ BUY, then place the 2×2 machine via `B` (250c).
2. Connect wires (`+`) from a burning generator so the BFS grid reaches the `Z` anchor.
3. Press `E` next to the machine → pick a recipe (radiobox) → **LOAD** (inputs are consumed from the backpack).
4. Wait 8 s (10 EU/tick). If the EU pool empties, the machine shows **PAUSED (no power)** and resumes when power returns.
5. **COLLECT** distributes all outputs into the correct backpack categories.

**中文：**
1. `T` ▸ BUY 购买蓝图，`B` 放置 2×2 机器（250c）。
2. 从燃烧中的发电机铺设电线（`+`），使 BFS 电网连通到 `Z` 锚点。
3. 机器旁按 `E` → 选择配方（Radiobox）→ **LOAD**（从背包扣除输入）。
4. 等待 8 秒（10 EU/tick）。EU 池耗尽时显示 **PAUSED (no power)**，恢复供电后续接进度。
5. **COLLECT** 将全部产物按分类发放到背包。

---

## Chemistry Bench / 化合台（`K`）

| Property / 属性 | Value / 数值 |
|-----------------|--------------|
| Footprint / 占地 | 2×2 — anchor `K` bright green, helpers `k` dark green / 锚点亮绿，辅助格暗绿 |
| Blueprint / 蓝图 | 180c (one-time / 一次性) |
| Placement / 放置费 | 120c (Build menu index 7 / 建造菜单第 7 项) |
| Power / 耗电 | **None — instant craft / 无——即时合成** |

### Synthesis recipes / 化合配方

| # | Recipe / 配方 | Input / 输入 | Output / 输出 |
|---|---------------|--------------|---------------|
| 1 | Synthesize Water / 合成水 | hydrogen ×2 + oxygen ×1 | water ×1 |
| 2 | Synthesize Salt / 合成盐 | sodium ×1 + chlorine ×1 | salt ×1 |
| 3 | Synthesize Sodium Hydroxide / 合成氢氧化钠 | sodium ×1 + oxygen ×1 + hydrogen ×1 | sodium_hydroxide ×1 |
| 4 | Synthesize Copper Sulfate / 合成硫酸铜 | copper ×1 + sulfur ×1 + oxygen ×4 | copper_sulfate ×1 |
| 5 | Synthesize Iron Sulfide / 合成硫化亚铁 | iron_ingot ×1 + sulfur ×1 | iron_sulfide ×1 |

**EN / 中文：** The recipe list shows live `have / need` per input in red (missing) or green (sufficient). **CRAFT** is rejected unless every input is available.
配方列表对每种输入实时显示 `have / need`，红色=不足、绿色=充足。任一输入不足时 **CRAFT** 被拒绝。

---

## Economy / 经济性

**EN:** Margins computed on sell value; BUY cost basis for feedstock (salt 45c, water 10c; bauxite_dust is self-produced at 22c opportunity cost).

**中文：** 按售价计算毛利；原料按 BUY 成本计（盐 45c、水 10c；铝土粉为自产，按 22c 机会成本计）。

| Chain / 链路 | In / 投入 | Out / 产出 | Margin / 毛利 |
|--------------|-----------|-----------|----------------|
| Water electrolysis / 水电解 | 10c | 85c | **+75c** |
| Molten-salt / 熔盐电解 | 45c | 110c | **+65c** |
| Chlor-alkali / 氯碱 | 55c | 200c | **+145c** |
| Alumina / 铝土电解 | 22c | 127c | **+105c** |
| Bench NaOH / 化合 NaOH | 115c | 120c | +5c |
| Bench CuSO₄ / 化合硫酸铜 | 188c | 220c | **+32c** |
| Bench FeS / 化合硫化亚铁 | 56c | 90c | **+34c** |

> **EN / 中文：** Water and salt *synthesis* are intentionally loss-making — they exist to reconvert elements, not to profit.
> 逆向*合成*水与盐刻意亏损——用途是元素转换，而非赚钱。

**Recommended money route / 推荐赚钱路线：** `T` buy salt+water → chlor-alkali (200c / 8s / 800 EU) → repeat. Feed extra water electrolysis for O₂ stockpiles.
`T` 买盐+水 → 氯碱（200c / 8 秒 / 800 EU）→ 循环；多余水电解囤积 O₂。

---

## Save & technical notes / 存档与技术说明

**EN:**
- Save format **v7**; blueprint line stores 8 flags, last two = electrolyzer / chemistry bench. v2–v6 load with both defaulting to `false`.
- Machine instances are *not* serialized; they are reconstructed from `machineMeta` (find-or-insert) + tile rehydration.
- Both `Z` and `K` (and their lowercase helpers) are conductive in the grid BFS, so power can pass *through* them. Only the Electrolyzer draws power.
- Helper functions: `isElement()`, `isCompound()`, `chemOutputCategory()` (ingots → `product`, new elements → `element`, compounds → `compound`, water → `basic`).

**中文：**
- 存档 **v7**；蓝图行 8 个标记，最后两个为电解机 / 化合台。v2–v6 加载时二者默认 `false`。
- 机器实例**不**序列化；通过 `machineMeta`（find-or-insert）+ 瓦片 rehydration 重建。
- `Z`、`K`（含小写辅助格）均在电网 BFS 导体集中，电可穿过机器传导；仅电解机耗电。
- 工具函数：`isElement()`、`isCompound()`、`chemOutputCategory()`（锭→`product`、新元素→`element`、化合物→`compound`、水→`basic`）。

---

## FAQ / 常见问题

**EN:**
- *Electrolyzer stuck at PAUSED?* Check the generator is burning and that wires 4-connect all the way to the `Z` anchor (BFS, no diagonals).
- *Where do I get sulfur?* Nowhere in v0.5.0 — it is reserved for a future update.
- *Why are elements missing from BUY?* Intentional: electrolysis is their only source, giving the machine chain value.

**中文：**
- *电解机一直 PAUSED？* 检查发电机是否在烧煤，电线是否 4 邻接一路铺到 `Z` 锚点（BFS，不支持对角）。
- *硫从哪来？* v0.5.0 暂无来源，预留给后续更新。
- *BUY 页为什么没有元素？* 刻意设计：电解是元素唯一来源，保证机器链价值。
