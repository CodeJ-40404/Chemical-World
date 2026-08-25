# Trading Market

交易市场详解。按 `T` 键打开。

---

## 🏪 界面结构

```
┌──────────────── $ TRADING MARKET $ ────────────────┐
│ Coins: 250                                         │
│                                                    │
│ TAB: [  SELL  ] [  BUY  ]  ← Radiobox 切换        │
├────────────────────────────────────────────────────┤
│ YOUR BACKPACK / BUY CATALOG  │  Item detail        │
│ ┌──────────────────────────┐ │  ┌────────────────┐│
│ │ hematite   x10   [5c]   │ │  │ Item: hematite ││
│ │ magnetite  x5    [5c]   │ │  │ Stock: x10     ││
│ │ coal       x8    [8c]   │ │  │ Price: 5c      ││
│ │ ...                     │ │  │ Total: 50c     ││
│ └──────────────────────────┘ │  └────────────────┘│
│                              │  [SELL ONE]        │
│                              │  [SELL ALL]        │
│                              │  [SELL EVERYTHING] │
│                              │  [CLOSE]           │
└────────────────────────────────────────────────────┘
Tip: UP/DOWN on TAB to switch SELL/BUY; ...
```

### 操作流程

1. **切 Tab**：聚焦顶部 Radiobox，按 ↑/↓ 在 SELL/BUY 间切换
2. **选条目**：Tab 跳到菜单，按 ↑/↓ 选择
3. **执行**：Tab 跳到按钮，回车触发
4. **退出**：按 `ESC` 或点 `CLOSE`

> 💡 用 Maybe 门控让非当前 tab 的菜单失活，确保焦点落到正确菜单上。

---

## 💰 SELL Tab（卖出）

显示背包中所有数量 > 0 的物品，价格为 `itemPrice(name)`。

### 按钮

| 按钮 | 行为 |
|------|------|
| `SELL ONE` | 卖出选中物品 ×1，得 `price` coins，+5 exp |
| `SELL ALL` | 卖出选中物品的全部数量，得 `price × qty` coins，+5×qty exp |
| `SELL EVERYTHING` | 清空背包中所有可售物品，统计总价与总经验 |
| `CLOSE` | 关闭交易界面 |

### 售价表（itemPrice）

| 物品 | 售价 | 物品 | 售价 |
|------|------|------|------|
| water | 3c | steel | 30c |
| hematite | 5c | glass | 15c |
| magnetite | 5c | iron_ingot | 12c |
| bauxite | 4c | alloy | 45c |
| cassiterite | 5c | steel_gear | 25c |
| malachite | 6c | steel_rod | 20c |
| chalcopyrite | 6c | steel_plate | 18c |
| gold_ore | 20c | steel_spring | 30c |
| silver_ore | 12c | steel_bolt | 12c |
| coal | 8c | steel_wire | 35c |
| sand | 2c | | |

不在表中的物品 `itemPrice` 返回 0，显示 `[not sellable]`。

---

## 🛒 BUY Tab（买入）

### 商品分类

#### 1. 原材料 / 燃料（14 种）

价格 = `itemPrice(name) × 3`，最低 10c。

| 物品 | 单价 |
|------|------|
| hematite | 15c |
| magnetite | 15c |
| bauxite | 12c |
| cassiterite | 15c |
| malachite | 18c |
| chalcopyrite | 18c |
| gold_ore | 60c |
| silver_ore | 36c |
| coal | 24c |
| sand | 6c → 最低 10c |
| glass | 45c |
| steel | 90c |
| iron_ingot | 36c |
| alloy | 135c |

#### 2. 钢零件（6 种）

价格 = `max(itemPrice(name) × 3, 30)`。

| 物品 | 单价 |
|------|------|
| steel_gear | 75c |
| steel_rod | 60c |
| steel_plate | 54c → 最低 30c |
| steel_spring | 90c |
| steel_bolt | 36c → 最低 30c |
| steel_wire | 105c |

#### 3. 蓝图（2 种，一次性）

| 蓝图 | 价格 | 解锁 |
|------|------|------|
| Generator Blueprint | 150c | BuildUI 中放置 Generator (2×2, 100c/台) |
| Wire Blueprint | 50c | BuildUI 中放置 Wire (1×1, 5c/根) |

### 蓝图机制

- **一次性购买**：买一次后永久解锁，永久显示 `[OWNED]` 标签
- **不进背包**：蓝图不是可堆叠物品，购买后直接设置 `gen_blueprint_unlocked = true` / `wire_blueprint_unlocked = true`
- **存档保存**：蓝图状态写入 v3 存档的 `genBP wireBP` 字段

### BUY 按钮

| 按钮 | 行为 |
|------|------|
| `BUY 1` | 买入选中物品 ×1，扣 `buyPrice` coins，+2 exp（蓝图 +15 exp） |
| `CLOSE` | 关闭交易界面 |

### 购买判定

```
if coins < buyPrice: "Not enough coins."
if blueprint && already owned: "You already own this blueprint."
else:
    coins -= buyPrice
    if blueprint: flag = true
    else: addItem(name, 1)
```

---

## 💡 策略建议

### 早期（< 200c）
- 挖 hematite / magnetite → 卖出积累启动资金
- 攒 150c + 50c = 200c 买两份蓝图
- 攒 100c 建第一台发电机

### 中期（200c ~ 1000c）
- 买 coal（24c）比挖省时间
- 4 槽并行烧 steel，30s 出 4 块
- 车床加工 steel_wire（35c 售价 ×3 = 105c 买价，差价大）

### 后期（> 1000c）
- 多发电机并联，连续加工
- 大量买 ore 自动化冶炼
- 卖 alloy（45c 售价）最赚钱

---

## ⚠️ 常见问题

**Q：BUY Tab 选不了？**
A：早期版本有过 bug，已修复。确保用 ↑/↓ 在顶部 TAB Radiobox 切换，再 Tab 到菜单。

**Q：蓝图买了找不到？**
A：蓝图不进背包，直接解锁。打开 B 建造模式，看 Generator / Wire 是否还显示 `[LOCKED]`。

**Q：SELL EVERYTHING 会卖掉 coal 吗？**
A：会。所有 `itemPrice > 0` 的物品都会被清空，包括 coal。注意保留燃料。
