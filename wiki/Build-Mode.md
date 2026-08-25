# Build Mode

建造模式详解。按 `B` 键打开（仅在家园）。

---

## 🏗️ 入口

- **按键**：`B`（仅 `currentArea == Home` 时可用）
- **退出**：`ESC` 或点 `CLOSE` 按钮

> 在荒原或矿洞按 B 会提示 "Can only build at home."

---

## 🖥️ 界面布局

```
┌──────────────── # BUILD MODE [B] # ────────────────┐
│                                                     │
│  PLACEABLES            ┌────────────────────────┐  │
│  ───────────           │ Coins: 250c            │  │
│  > Generator (2x2,100c)│ EU Pool: 6400 / 10000   │  │
│    Wire (1x1, 5c)      │                        │  │
│    Flower (1c)         │ Blueprints:             │  │
│    Grass tuft (1c)     │  Generator BP: OWNED   │  │
│                        │  Wire BP:      LOCKED   │  │
│                        └────────────────────────┘  │
│                                                     │
│         ┌─────┬─────┬─────┬─────┬─────┐            │
│         │     │     │     │     │     │            │
│         ├─────┼─────┼─────┼─────┼─────┤            │
│         │     │     │     │     │     │            │
│         ├─────┼─────╔═════╗─────┼─────┤            │
│         │     │     ║  P  ║     │     │  ← 双线框  │
│         ├─────┼─────╚═════╝─────┼─────┤    = 玩家  │
│         │     │     │     │     │     │    脚下    │
│         ├─────┼─────┼─────┼─────┼─────┤            │
│         │     │     │     │     │     │            │
│         └─────┴─────┴─────┴─────┴─────┘            │
│                                                     │
│              [PLACE]    [CLOSE]                    │
│                                                     │
│  Tip: LEFT item pick type; PLACE puts it at center │
└─────────────────────────────────────────────────────┘
```

---

## 📐 7×7 放大网格

- **范围**：以玩家脚下为中心的 7×7 区域（dx, dy ∈ [-3, +3]）
- **中心格**：双线边框 `DOUBLE`，标识"放置位置"（玩家当前脚下）
- **其他格**：单线边框 `LIGHT`
- **颜色**：根据 tile display 字符着色

### 颜色映射

| 字符 | 含义 | 颜色 |
|------|------|------|
| `.` | 草地 | 亮绿 |
| `~` | 河流 / 湖泊 | 亮蓝 |
| `*` | 花朵 | 亮紫 |
| `v` | 草丛 | 绿 |
| `t` | 树 | 绿 |
| `+` | 电线 | 亮黄 |
| `G` / `g` | 发电机 | 亮黄 |
| `F` / `f` | 高炉 | 亮红 |
| `L` / `l` | 车床 | 紫 |
| `C` | Car | 黄 |
| `#` | 边界 | （显示为空格） |

---

## 🛠️ 可放置物

| 序号 | 名称 | 尺寸 | 价格 | 蓝图要求 | 字符 |
|------|------|------|------|----------|------|
| 0 | Generator | 2×2 | 100c | Generator BP | `G` + `g` |
| 1 | Wire | 1×1 | 5c | Wire BP | `+` |
| 2 | Flower | 1×1 | 1c | 无 | `*` |
| 3 | Grass tuft | 1×1 | 1c | 无 | `v` |

### 锁定状态

未购买蓝图时，菜单项显示 `[LOCKED: buy BP first]`，PLACE 会提示：
```
Generator blueprint required. Buy it from Trade > BUY.
```

---

## 🎯 PLACE 操作

按下 `PLACE` 时，根据当前选中：

### 0. Generator

```cpp
if (!gen_blueprint_unlocked) → "Generator blueprint required."
if (coins < 100) → "Need 100 coins."
if (placeGenerator(px, py)) → "Generator installed at (x,y)."
else → "Space is already occupied or impassable."
```

`placeGenerator(x, y)`：
1. 检查 4 格 `(x,y) (x+1,y) (x,y+1) (x+1,y+1)` 全部 passable
2. 检查 `gen_blueprint_unlocked && coins >= 100`
3. 扣 100 coins
4. 写入 4 格：`(x,y)='G'` 黄色 / 其余 3 格 `'g'` 暗黄
5. 4 格全部设为 `passable = false`
6. `machineMeta.push_back({x, y, 'G', 0, 0, false})`
7. `generators[{x, y}] = PowerGenerator{}`

### 1. Wire

```cpp
if (!wire_blueprint_unlocked) → "Wire blueprint required."
if (coins < 5) → "Need 5 coins."
if (placeWire(px, py)) → "Wire laid at (x,y)."
else → "Cannot lay wire here."
```

`placeWire(x, y)`：
1. 检查 `(x, y)` passable
2. 检查 `wire_blueprint_unlocked && coins >= 5`
3. 扣 5 coins
4. 写入 `'+'` 蓝色，`passable = false`

### 2 / 3. Flower / Grass tuft

```cpp
if (coins < 1) → "Need 1 coin."
if (placeDecor(px, py, '*')) → "Planted a flower at (x,y)."
else → "Blocked."
```

`placeDecor(x, y, display)`：
1. 检查 `(x, y)` passable
2. 扣 1 coin
3. 写入 `display`（`'*'` 紫色随机 / `'v'` 深绿色）
4. **保持 `passable = true`**（装饰可踩）

---

## ⚡ 100ms Ticker

BuildUI 内启动一个 ticker 线程，每 100ms 调用 `globalTick100ms()`，会：
- 推进电力网络（`tickPowerGrid`）
- 推进土高炉（`furnace.update`）
- 推进车床（`lathe.update`，含电力检查）

所以**在 BuildUI 里也能看到 EU Pool 实时变化**（投了煤的发电机持续注入）。

---

## 📋 完整建造流程示例

### 目标：在家园放一台发电机并连到车床

```
1. T → BUY → Generator Blueprint (150c) → BUY 1
2. T → BUY → Wire Blueprint (50c) → BUY 1
3. B 打开建造模式
4. 走到 (15, 5) 位置
5. B 重新打开，菜单选 Generator，PLACE
   → 4 格 (15,5)-(16,6) 变成 G/g，扣 100c
6. 走到 (14, 5)，菜单选 Wire，PLACE → 扣 5c
7. 走到 (13, 5)，PLACE Wire → 扣 5c
8. 重复直到 (11, 5) 都铺上 +
9. ESC 退出
10. 走到发电机 (15,5) 旁，按 E → 投煤
11. EU pool 开始上升
12. 走到车床 (8,5) 旁，按 E → 加工
```

电线连通图：
```
(15,5)G  ←──  (14,5)+  ←──  (13,5)+  ←──  (12,5)+  ←──  (11,5)+  ←──  (10,5)@player
                                                                          ↓
                                                                  (8,5)L lathe 锚点
```

> 注意 (10,5) 是玩家出生点，玩家离开后该格变回 `.`，电线需要铺到 (9,5) 才能连通到 L 锚点 (8,5)。

---

## ⚠️ 常见问题

**Q：为什么放不下 Generator？**
A：2×2 机器需要 4 格全空。检查放大网格中心十字区域是否都是 `.`（草地）。

**Q：电线铺到玩家身上会怎样？**
A：玩家位置是动态的，PLACE 时玩家会移动，电线会铺在玩家**脚下当时的位置**。

**Q：可以拆掉机器 / 电线吗？**
A：目前**不支持拆除**。放置前请确认位置。

**Q：装饰和电线混在一起会导电吗？**
A：不会。BFS 只通过 `G g F f L l +` 这 6 个字符。`*` 和 `v` 是绝缘体。
