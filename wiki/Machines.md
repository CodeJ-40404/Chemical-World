# Machines

机器详解：土高炉、车床、火力发电机。

---

## 🔥 土高炉（BlastFurnace, F）

**位置**：家园 (5,5) 2×2 区域（`F` 锚点 + `f` 三辅助格）

### 关键特性

| 属性 | 数值 |
|------|------|
| 槽数 | 4（并行） |
| 每批耗时 | 30 秒（30000 ms） |
| 电力需求 | **无** |
| 动画 | 24 帧 ASCII（STOKING / HEATING / POURING 三阶段） |
| 字符 | `F` 红色 / `f` 暗红 |
| 通行 | 不可通行 |

### 配方表（8 种）

| 配方名 | 输入 ×2 | 输出 | 输出数量 |
|--------|--------|------|---------|
| Steel Making | hematite | steel | 1 |
| Steel Making | magnetite | steel | 1 |
| Aluminum Smelt | bauxite | aluminum | 1 |
| Tin Smelting | cassiterite | tin | 1 |
| Copper Smelt | malachite | copper | 1 |
| Copper Smelt | chalcopyrite | copper | 1 |
| Gold Smelting | gold_ore | gold_ingot | 1 |
| Silver Smelt | silver_ore | silver_ingot | 1 |

> 每个配方需要 **2 个矿石 + 1 个 coal**（在 `update()` 内部消耗，UI 显示为配方名）

### 24 帧动画分段

```
progress 0%  ───────── 33% ─────────── 83% ─────── 100%
            STOKING 0-5      HEATING 6-17    POURING 18-23
            (铲煤入炉)         (烧炼)          (倒铁水)
```

- 帧 0-5：工人铲煤动作（`\\ /` `>#<` `/[\` 等）
- 帧 6-17：炉内火焰渐强（`*` `**` `***` 渐多）
- 帧 18-23：铁水倾倒（`~~~` 流出）
- DONE 状态：停在第 23 帧

### 操作流程

1. 按 `E` 在 F 旁打开 FurnaceUI
2. 顶部 4 个 Radiobox 切换槽（Slot 0-3）
3. 左侧 Menu 选 8 种配方之一
4. 点 `LOAD SLOT` 把当前槽装载该配方
5. 自动开始 30s 烧炼
6. 完成后点 `COLLECT` 取走产物
7. `CANCEL SLOT` 中止当前槽
8. 4 槽独立运行，可同时烧 4 种不同金属

> 💡 30s × 4 槽并行 = 平均 7.5s / 件，比 C 合成快得多。

---

## ⚙️ 车床（Lathe, L）

**位置**：家园 (8,5) 2×2 区域（`L` 锚点 + `l` 三辅助格）

### 关键特性

| 属性 | 数值 |
|------|------|
| 模具数 | 6 |
| 电力需求 | **2 EU / tick**（Machining + Inserting 状态） |
| 字符 | `L` 紫色 / `l` 暗紫 |
| 通行 | 不可通行 |

### 模具表

| 模具 | 输入 | 输出 | 输出量 | 耗时 | 经验 | 售价 |
|------|------|------|--------|------|------|------|
| Gear | steel ×1 | steel_gear | 1 | 3.0s | +12 | 25c |
| Rod | steel ×1 | steel_rod | 1 | 2.5s | +10 | 20c |
| Plate | steel ×1 | steel_plate | 1 | 2.0s | +8 | 18c |
| Spring | steel ×1 | steel_spring | 1 | 3.5s | +14 | 30c |
| Bolt | steel ×1 | steel_bolt | 2 | 1.5s | +6 | 12c |
| Wire | steel ×1 | steel_wire | 3 | 4.0s | +16 | 35c |

### 状态机

```
Idle ──LOAD──> Inserting (5 帧 / ~1s) ──> Machining ──完成──> Done
   ▲                                                          │
   └───────────────────COLLECT─────────────────────────────────┘
```

- **Idle**：显示选中模具的 icon
- **Inserting**：钢条 `[====]` 从左滑入卡盘（5 帧）
- **Machining**：加工动画循环（铁屑 + 火花 4 帧循环，200ms/帧）
- **Done**：显示 "JOB COMPLETE"，按 COLLECT 取产物

### 电力消耗

- Machining / Inserting 状态每 tick（100ms）扣 2 EU
- 没扣到 → `hasPowerThisTick = false`
- 车床自动暂停：`isRunning = false`，进度条变灰，显示 ⚠ PAUSED (no power)
- 恢复供电 → 自动恢复加工，日志显示 ⚡ Power restored

### 操作流程

1. 走到 L 旁按 `E` 打开 LatheUI
2. Idle 状态下左侧选模具
3. 点 `LOAD` 装载（消耗 1 个 steel）
4. 自动进入 Inserting → Machining
5. 期间发电机必须有电
6. 完成后点 `COLLECT` 取走产物
7. `CANCEL` 中止加工

---

## ⚡ 火力发电机（PowerGenerator, G）

**位置**：玩家通过 B 键建造模式放置（需要 Generator Blueprint）

### 关键特性

| 属性 | 数值 |
|------|------|
| 尺寸 | 2×2 格 |
| 单价 | 100 coins（不含蓝图） |
| 蓝图价格 | 150 coins（一次性，BUY tab 购买） |
| 单煤发电 | 6400 EU |
| 燃烧速度 | 8 EU / tick |
| 单煤燃烧时间 | 80 秒 |
| 字符 | `G` 黄色 / `g` 暗黄 |
| 通行 | 不可通行 |

### 状态

- **BURNING**：`burnEU > 0`，每 tick 注入 ≤8 EU 到 pool，`active = true`
- **IDLE**：`burnEU = 0`，`active = false`

### GeneratorPanel（E 键打开）

显示：
- 剩余燃烧 EU 数值条（40 段，满 = 6400）
- 状态：BURNING（绿）/ IDLE（红）
- `ADD 1 COAL` 按钮：消耗背包 1 个 coal，`burnEU += 6400`
- `CLOSE` 按钮

---

## 📐 机器 2×2 锚点约定

所有 2×2 机器使用 **大写字母锚点 + 小写辅助格** 约定：

```
F f     L l     G g
F f     L l     G g
↑       ↑       ↑
锚点     锚点     锚点
(左上)  (左上)  (左上)
```

- 锚点（左上格）使用大写字母 `F/L/G`，机器元信息 `machineMeta` 只记录锚点坐标
- 其余 3 格使用小写 `f/l/g`，仅作显示
- `isNear(x, y, 'F', px, py)` 只匹配大写锚点，避免误判
- 电线 `+` 是 1×1，没有锚点概念

### 默认布局（新游戏）

```
machineMeta = [
    { x:5, y:5, type:'F' },  // BlastFurnace
    { x:8, y:5, type:'L' },  // Lathe
]
generators = {}  // 玩家需自己建造
```
