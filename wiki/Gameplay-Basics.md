# Gameplay Basics / 玩法基础

> **EN:** New-player guide: spawn, the three maps, mining, crafting, progression.
> **中文：** 新手指南：出生、三大地图、挖矿、合成与成长路线。

---

## 🌍 Spawn & maps / 出生与地图

**EN:** A new game spawns you at **Home (10,5)** with:
**中文：** 新游戏出生在**家园 (10,5)**，初始物品：

| Item / 物品 | Qty / 数量 | Purpose / 用途 |
|------|------|------|
| water | 5 | Chemistry feedstock (BASIC / RAW tab) / 化学原料（BASIC / RAW 页） |
| coal | 5 | Furnace & generator fuel / 高炉与发电机燃料 |
| sand | 3 | Sellable raw material (glass is bought at the market) / 可卖原料（glass 在市场购买） |

| Area / 区域 | Size / 尺寸 | Entry / 入口 | Features / 特点 |
|------|------|------|------|
| **Home 家园** | 60×50 | spawn 出生 | River/lake, flowers, Furnace `F` (5,5), Lathe `L` (8,5), Car `C` / 河流湖泊、高炉、车床、Car |
| **Wasteland 荒原** | 150×80 | `E` at the Car | 8 biomes, widespread ores, cave entrance `O` / 8 群系、矿石、矿洞入口 |
| **Cave 矿洞** | 45×28 | `E` at `O` in Wasteland | Dense ores; exit via `O` / 密集矿石，`O` 离开 |

**EN:** Fixed Home layout:
**中文：** 家园固定布局：

```
F Furnace (5,5)     L Lathe (8,5)        @ spawn (10,5)      C Car (12,5)
```

**EN:** Decor generated at Home: rivers/lakes `~` (impassable), 40 flowers `*`, 30 grass tufts `v` (both passable).
**中文：** 家园自动生成：河流/湖泊 `~`（不可通行）、40 朵花 `*`、30 丛草 `v`（均可通行）。

---

## ⛏️ Mining / 挖矿

**EN:** Find an ore tile in Wasteland/Cave, stand orthogonally adjacent, press `E` — each ore needs **3 hits**, then drops into the backpack.
**中文：** 在荒原/矿洞找到矿石，站到上下左右相邻格按 `E`——每块矿需**敲 3 次**，产物自动入背包。

| Ore / 矿物 | Sell / 售价 | Smelts to / 冶炼产物 |
|------|------|----------|
| hematite 赤铁矿 | 5c | steel |
| magnetite 磁铁矿 | 5c | steel |
| bauxite 铝土矿 | 4c | aluminum |
| cassiterite 锡石 | 5c | tin |
| malachite 孔雀石 | 6c | copper |
| chalcopyrite 黄铜矿 | 6c | copper |
| gold_ore 金矿 | 20c | gold_ingot |
| silver_ore 银矿 | 12c | silver_ingot |
| coal 煤 | 6c | used directly / 直接使用 |
| sand 沙 | 1c | glass |

---

## 🏭 Processing chains / 加工链

**EN (v0.5.0):**

```
ore ──► Furnace F (no power, 30s×4) ──► ingots ──► Lathe L (2 EU) ──► steel parts
 │
 └──► Crusher X (4 EU) ► Washer W (2 EU) ► Centrifuge R (8 EU) ──► dusts ─┐
                                                                           │
   salt/water (BUY) ──► Electrolyzer Z (10 EU, 8s) ──► elements ──► Chemistry Bench K ──► compounds
                          ▲ bauxite_dust feeds alumina recipe             (no power, instant)
```

**中文（v0.5.0）：**

```
矿石 ──► 土高炉 F（无电，30s×4）──► 锭 ──► 车床 L（2 EU）──► 钢零件
 │
 └──► 破碎机 X（4 EU）► 洗矿槽 W（2 EU）► 离心机 R（8 EU）──► 矿粉 ─┐
                                                                    │
 盐/水（BUY）──► 电解机 Z（10 EU，8s）──► 元素 ──► 化合台 K ──► 化合物
                  ▲ 铝土粉喂入铝土电解配方              （无电，即时）
```

➡️ **EN:** Chemistry details, recipes and profit tables: [Chemistry](Chemistry.md)
**中文：** 化学详解、配方与收益表：[化学系统](Chemistry.md)

---

## 🔨 Crafting & backpack / 合成与背包

**EN:** Press `C` to open the **backpack** (the early standalone steel/glass craft menu was retired — steel comes from the Blast Furnace, which consumes 2 ore + 1 coal per batch internally; `glass` is bought at the market). The Chemistry Bench `K` is the current synthesis station.
**中文：** 按 `C` 打开**背包**（早期的独立钢/玻璃合成菜单已移除——钢由土高炉产出，每批内部消耗 2 矿石 + 1 煤；`glass` 在市场购买）。当前的合成站是化合台 `K`。

### Backpack tabs / 背包标签页（v0.5.0）

`ORES/MINERALS` · `BASIC/RAW` · `FUELS` · `MATERIALS` · `PRODUCTS` · **`ELEMENTS`** · **`COMPOUNDS`** · `GEMS` · `BLUEPRINTS` · `MISC`

---

## 📈 XP & leveling / 经验与等级

| Action / 行为 | XP / 经验 |
|---|---|
| Sell item / 卖物品 | +5 each |
| Buy blueprint / 买蓝图 | +15 |
| Craft / 合成 | +8–10 |

**EN:** Level-up threshold = `100 + level × 20`.
**中文：** 升级阈值 = `100 + 等级 × 20`。

---

## 🎓 Tutorial / 教程

**EN:** 5 guided steps on first run: Move → Collect → Craft → Trade → Blast Furnace. Old saves never re-trigger tutorial steps.
**中文：** 首次游戏 5 步引导：移动 → 采集 → 合成 → 交易 → 高炉。旧存档不会重新触发教程。
