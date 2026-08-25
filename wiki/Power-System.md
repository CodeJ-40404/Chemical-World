# Power System

电力网络详解。灵感来自 GT:New Horizons 的简化 EU 模型。

---

## ⚡ 核心概念

游戏采用**单一共享 EU 池**模型：

```
            ┌─────────────────────────────┐
            │   global EU Pool (0~10000)  │
            └─────────────────────────────┘
                  ▲              │
        注入 ≤8/tick             │ 扣减 (powerDraw)
                  │              ▼
        ┌─────────────────┐  ┌──────────────┐
        │ 火力发电机 G     │  │ 车床 L (2EU) │
        │ (burning coal)  │  │              │
        └─────────────────┘  └──────────────┘
                  ▲              │
                  │   电线 +    │
                  └─── connects ┘
```

- **EU pool**：全局变量 `globalEU`，范围 0~10000，所有发电机共享
- **发电机**：投煤后燃烧，每 tick（100ms）注入 ≤8 EU 到 pool
- **机器**：每 tick 从 pool 扣 EU，扣不到就暂停
- **电线**：连接发电机与机器，决定哪些机器"通电"

---

## 🔥 火力发电机

| 属性 | 数值 |
|------|------|
| 尺寸 | 2×2 格（`G` 锚点 + `g` 三个辅助格） |
| 单次投煤 | 充 6400 EU |
| 燃烧速度 | 8 EU / tick（100ms） |
| 单煤总燃烧时间 | 6400 / 8 × 100ms = **80 秒** |
| 颜色 | `G` 黄色 / `g` 暗黄 |
| 通行 | 不可通行（4 格都阻挡） |

### 投煤方法

1. 走到发电机 2×2 区域旁（任何一格的上下左右 4 邻接）
2. 按 `E`，弹出 GeneratorPanel
3. 点击 `ADD 1 COAL` 按钮
4. 消耗背包中 coal ×1，发电机 `burnEU += 6400`，`active = true`

### 多发电机并联

可以放置多台发电机，全部燃烧时总注入速度叠加：
- 1 台燃烧：+8 EU/tick
- 2 台燃烧：+16 EU/tick
- N 台燃烧：+N×8 EU/tick

EU pool 上限 10000，满了就停止注入（不浪费）。

---

## 🔌 电线网络

| 属性 | 数值 |
|------|------|
| 尺寸 | 1×1 格 |
| 单价 | 5 coins |
| 字符 | `+`（蓝色） |
| 通行 | 不可通行 |
| 连通规则 | 4 邻接（上下左右），无损耗 |

### 导电性判定（BFS）

每个 tick，`tickPowerGrid()` 会：

1. 找到所有发电机 2×2 锚点
2. 从每个锚点开始 BFS，**通过所有导电字符**：
   ```
   G g F f L l +
   ```
3. 所有 BFS 能到达的机器锚点加入 `poweredMachines` 集合
4. 调用 `powerDraw(eu, machineXY)` 时检查：机器锚点必须在 `poweredMachines` 里

> ⚠️ 注意：电线本身**不是机器**，只是导体。BFS 会从 G 出发经过 `+` 一直走到 F 或 L 锚点。

---

## 🎯 powerDraw 流程

```
powerDraw(eu, machineXY):
  if currentArea != Home: return false
  if machineXY not in poweredMachines: return false
  if globalEU < eu: return false
  globalEU -= eu
  return true
```

车床每 tick（Machining/Inserting 状态）调用：
```cpp
lathe.hasPowerThisTick = powerDraw(2, {8, 5});
```

- 成功扣到 → `hasPowerThisTick = true`，车床正常推进
- 扣不到 → `hasPowerThisTick = false`，车床进入 paused 状态，进度条变灰，显示 ⚠ PAUSED

---

## 🏗️ 完整电力链示例

```
玩家出生 (10,5)
  ↓
  T 买 Wire Blueprint (50c) + Generator Blueprint (150c)
  ↓
  B 进入建造模式
  ↓
  在 (15,5) 放 Generator (2×2: G/g, 消耗 100c)
  ↓
  沿 (14,5) (13,5) (12,5) (11,5) 铺设 Wire（每个 5c, 共 20c）
  ↓
  连到 Lathe 锚点 (8,5)
  ↓
  E 走到 G 旁, 投 1 个 coal (burnEU = 6400)
  ↓
  E 走到 L 旁, 加工 steel_gear (3s = 30 ticks × 2 EU = 60 EU)
  ↓
  EU pool 持续被扣，发电机持续注入，加工完成
```

---

## 📊 EU 预算参考

| 操作 | 持续时间 | 总 EU 消耗 |
|------|----------|-----------|
| 加工 steel_gear (3s) | 30 ticks | 60 EU |
| 加工 steel_rod (2.5s) | 25 ticks | 50 EU |
| 加工 steel_wire (4s) | 40 ticks | 80 EU |
| 1 块煤发电 | 80s | 注入 6400 EU |

> 1 块煤可加工约 100+ 个钢零件，电力通常不紧张。

---

## ⚠️ 常见问题

**Q：车床一直 PAUSED 怎么办？**
A：检查 GeneratorPanel 看是否还在 burning。投煤、确认电线连通到 Lathe 锚点 (8,5)。

**Q：发电机燃烧但车床还是没电？**
A：电线必须 4 邻接连通，且最终能 BFS 到达 L 锚点。中间断一格就不行。

**Q：可以一个发电机带多台车床吗？**
A：可以。EU pool 是共享的，只要总消耗 < 总注入就不会断电。
