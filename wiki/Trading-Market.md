# Trading Market / 交易市场

> **EN:** Press `T` anywhere to open the SELL/BUY dual-tab market.
> **中文：** 任意位置按 `T` 打开 SELL/BUY 双标签交易市场。

**Tabs / 标签：** ↑/↓ on the top radiobox · **Buttons / 按钮：** Tab to focus, Enter to activate · **Exit / 退出：** `ESC`

---

## 💰 SELL tab / 卖出页

**EN:** Lists every backpack item with qty > 0; sell prices come from `itemPrice(name)`. Items priced 0 show `[not sellable]`.
**中文：** 列出背包中数量 > 0 的全部物品；售价来自 `itemPrice(name)`。价格为 0 显示 `[not sellable]`。

| Button / 按钮 | EN / 中文 |
|------|------|
| SELL ONE | Sell ×1, +5 XP / 卖 1 个，+5 经验 |
| SELL ALL | Sell full stack, +5×qty XP / 卖整组 |
| SELL EVERYTHING | Sell every sellable item (watch your coal!) / 清空所有可售物品（小心把煤卖了！） |

### Notable sell prices / 主要售价

| Category / 分类 | Item / 物品 (sell / 售价) |
|---|---|
| Raw / 原料 | water 2 · sand 1 · coal 6 · bauxite 4 · hematite 5 · malachite 6 · silver_ore 12 · gold_ore 20 |
| Products / 产物 | iron_ingot 16 · steel 22 · aluminum 26 · copper 48 · tin 95 · silver_ingot 600 · gold_ingot 1500 |
| Dusts / 矿粉 | bauxite_dust 22 · hematite_dust 18 · magnetite_dust 18 |
| **Elements / 元素** | oxygen 25 · hydrogen 30 · nitrogen 35 · sulfur 40 · magnesium 45 · silicon 55 · chlorine 50 · sodium 60 |
| **Compounds / 化合物** | water 2 · salt 15 · iron_sulfide 90 · sodium_hydroxide 120 · copper_sulfate 220 |
| Steel parts / 钢零件 | bolt 12 · plate 18 · rod 20 · gear 25 · spring 30 · wire 35 |

> **EN / 中文：** Metal ingots are *also* chemistry elements but keep their PRODUCT category, so furnace and electrolyzer output merges into one stack.
> 金属锭同时也是化学元素，但保留 PRODUCT 分类，使炉子与电解机产出合并为同一堆叠。

---

## 🛒 BUY tab / 买入页

**EN:** Three catalogs. Raw materials cost `itemPrice × 3` (minimum 10c); steel parts `max(price×3, 30c)`; blueprints are one-time unlocks.
**中文：** 三类商品。原材料 = `售价 × 3`（最低 10c）；钢零件 = `max(售价×3, 30c)`；蓝图为一次性解锁。

### 1. Raw materials / 原材料（16）

**EN:** hematite, magnetite, bauxite, cassiterite, malachite, chalcopyrite, gold_ore, silver_ore, coal, sand, glass, steel, iron_ingot, alloy, **salt (45c)**, **water (10c)**.
**中文：** 8 种矿石 + coal、sand、glass、steel、iron_ingot、alloy，以及 **salt（45c）**、**water（10c）**。

> **EN:** Examples: salt 15×3 = 45c · water 2×3 = 6 → floored to 10c · coal 18c… (6×3=18) · iron_ingot 48c (16×3).
> **中文：** 例：盐 15×3=45c；水 2×3=6 → 保底 10c；iron_ingot 16×3=48c。

### 2. Steel parts / 钢零件（6）

gear 75c · rod 60c · plate 54c · spring 90c · bolt 36c · wire 105c

### 3. Blueprints / 蓝图（8, one-time / 一次性）

| Blueprint / 蓝图 | Price / 价格 | Unlocks / 解锁 |
|---|---|---|
| Generator Blueprint | 150c | Generator placement (100c/台) |
| Wire Blueprint | 50c | Wire placement (5c/根) |
| Crusher Blueprint | 200c | `X` (150c/台) |
| Ore Washer Blueprint | 160c | `W` (120c/台) |
| Centrifuge Blueprint | 250c | `R` (200c/台) |
| Gem Sorter Blueprint | 220c | `S` (180c/台) |
| **Electrolyzer Blueprint** | **300c** | `Z` (250c/台) |
| **Chemistry Bench Blueprint** | **180c** | `K` (120c/台) |

**Blueprint mechanics / 蓝图机制：**
- **EN:** Buy once → permanent `[OWNED]`; not a backpack item (sets a save flag); BUY grants +15 XP.
- **中文：** 一次购买→永久 `[OWNED]`；不进背包（写存档标记）；购买 +15 经验。
- **EN:** Raw elements (H/O/Cl/Na/…) are deliberately **not for sale** — Electrolyzer output is their only source.
- **中文：** 元素（H/O/Cl/Na 等）刻意**不出售**——电解是唯一来源。

---

## 💡 Strategy / 策略建议

| Stage / 阶段 | EN | 中文 |
|---|---|---|
| Early 早期 (<200c) | Sell ores; buy Generator + Wire BPs (200c total) | 卖矿攒钱，买发电机+电线蓝图 |
| Mid 中期 (200–1000c) | Furnace batches + lathe wire; processing chain | 高炉批量+车床 wire，上处理链 |
| Late 后期 (>1000c) | **Electrolyzer chlor-alkali: 55c in → 200c out / 8 s** | **氯碱电解：55c 进 → 200c 出 / 8 秒** |

**EN:** Full chemistry margin table: [Chemistry](Chemistry.md)#economy.
**中文：** 化学收益全表：[化学系统](Chemistry.md)。

---

## ⚠️ FAQ / 常见问题

**EN / 中文：**
- *Can't focus BUY list?* Switch the top SELL/BUY radiobox first (↑/↓), then Tab into the list. / 先切顶部 Radiobox，再 Tab 进列表。
- *Bought blueprint, can't find it?* Blueprints never enter the backpack — open `B`, the lock label is gone. / 蓝图不进背包——打开 `B` 可见锁消失。
- *SELL EVERYTHING sold my coal?* By design — every itemPrice > 0 is sold. / 刻意设计：售价 >0 的物品都会被卖。
