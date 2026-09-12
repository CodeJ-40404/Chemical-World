# Power System / 电力系统

> **EN:** Simplified GT:NH-style EU model: thermal generators inject energy into one shared 0–10,000 EU pool; wires decide *which* machines are connected via a per-tick BFS; machines draw from the pool and pause when they cannot.
> **中文：** 简化的 GT:NH 风格 EU 模型：发电机向一个 0–10,000 的共享 EU 池注入能量；每 tick 的 BFS 决定哪些机器与电网连通；机器从池中扣电，扣不到就暂停。

---

## ⚡ Core model / 核心模型

```
            ┌─────────────────────────────┐
            │   global EU Pool (0~10000)  │
            └─────────────────────────────┘
                  ▲              │
        inject ≤8/tick           │ powerDraw(eu, xy)
                  │              ▼
        ┌─────────────────┐  ┌──────────────────────────┐
        │ Generator G     │  │ L 2 · W 2 · X 4/slot     │
        │ (burns coal)    │  │ S 6 · R 8 · Z 10 EU/tick │
        └─────────────────┘  └──────────────────────────┘
                  ▲              │
                  └── wires + + machines are conductive (BFS) ──┘
```

| Concept / 概念 | EN | 中文 |
|----------------|----|------|
| EU pool | `globalEU`, shared by every generator, capped at 10,000 | `globalEU`，所有发电机共享，上限 10,000 |
| Generator | burns one coal = 6,400 EU, injects ≤8 EU/tick (100 ms) | 每煤 6,400 EU，每 tick 注入 ≤8 EU |
| Machine | calls `powerDraw` while active; failure → paused | 活跃时调用 `powerDraw`；失败→暂停 |
| Wire | 4-connected conductors; no distance loss | 4 邻接导体；无距离损耗 |

---

## 🔥 Generator / 火力发电机（G）

| Property / 属性 | Value / 数值 |
|------|------|
| Size / 占地 | 2×2 (`G` anchor + 3 `g` helpers) |
| Cost / 成本 | BP 150c + 100c per unit / 蓝图 150c + 每台 100c |
| Per coal / 每煤 | 6,400 EU, 8 EU/tick → **80 s** burn time / 燃烧 80 秒 |
| Passable / 通行 | no (all 4 tiles block) / 不可 |

**EN:** `E` nearby → `ADD 1 COAL`. N burning generators inject N×8 EU/tick; injection stops at the 10,000 cap (nothing is wasted).
**中文：** 机器旁按 `E` → `ADD 1 COAL`。N 台燃烧中的发电机注入 N×8 EU/tick；池满 10,000 即停注（不浪费）。

---

## 🔌 Wire network / 电线网络

| Property / 属性 | Value / 数值 |
|------|------|
| Size / 尺寸 | 1×1 |
| Cost / 单价 | BP 50c + 5c per wire / 蓝图 50c + 每根 5c |
| Char / 字符 | `+` (blue / 蓝) |
| Connectivity / 连通 | 4-neighbour, no loss / 4 邻接，无损耗 |

### Conductivity BFS / 导电性 BFS

**EN:** Every tick `tickPowerGrid()` (1) seeds the BFS from every generator's 4 tiles, (2) floods through conductive tiles, (3) registers each reachable machine anchor into `poweredMachines`.

**中文：** 每 tick `tickPowerGrid()`：(1) 以每台发电机 4 格作为 BFS 起点，(2) 沿导电字符泛洪，(3) 将可达的机器锚点注册进 `poweredMachines`。

Conductive character set / 导电字符集：

```
+  G g  F f  L l  X x  W w  R r  S s  Z z  K k
```

| Class / 类别 | Chars / 字符 | Registered for powerDraw / 注册耗电 |
|--------------|--------------|--------------------------------------|
| Wire / 电线 | `+` | no (conductor only / 仅导体) |
| Generator / 发电机 | `G`/`g` | self-powered, seeds BFS / 自发电，BFS 起点 |
| No-power machines / 不耗电机器 | `F`/`f` Furnace, `K`/`k` Chem Bench | conductive but never draw / 导电但不扣电 |
| Powered machines / 耗电机器 | `L` Lathe, `X` Crusher, `W` Washer, `R` Centrifuge, `S` Sorter, `Z` Electrolyzer | yes / 是 |

> **EN / 中文：** Electricity passes *through* machines, so a chain of adjacent machines needs no wire between them. Decor `*`/`v` and ore chars are insulators.
> 电可以穿过机器传导，相邻机器串之间无需补线。装饰 `*`/`v` 与矿石字符是绝缘体。

---

## 🎯 powerDraw semantics / powerDraw 流程

```cpp
powerDraw(eu, machineXY):
  if currentArea != Home:      return false;  // grid exists only at Home / 电网仅存在于家园
  if machineXY not in poweredMachines: return false;
  if globalEU < eu:            return false;  // pool drained / 电量不足
  globalEU -= eu;
  return true;
```

**EN:** Draw values per tick (100 ms) while processing:
**中文：** 加工中每 tick（100ms）扣电量：

| Machine / 机器 | Draw / 扣电 |
|----------------|-------------|
| Lathe `L` | 2 EU |
| Ore Washer `W` | 2 EU |
| Crusher `X` | 4 EU **per active slot** / 每活跃槽 |
| Gem Sorter `S` | 6 EU |
| Centrifuge `R` | 8 EU |
| **Electrolyzer `Z`** | **10 EU** |
| Chemistry Bench `K` | 0 (instant craft / 即时合成) |

**EN:** On failed draw `hasPowerThisTick=false`; the machine's `update()` skips progress and the UI shows `PAUSED (no power)`. Progress is retained and resumes when power returns.
**中文：** 扣电失败则 `hasPowerThisTick=false`；机器 `update()` 不推进进度，界面显示 `PAUSED (no power)`。进度保留，恢复供电后续跑。

---

## 📊 EU budget / EU 预算参考

| Operation / 操作 | Duration / 时长 | Total EU / 总耗电 |
|------|----------|-----------|
| steel_gear (lathe / 车床) | 3 s | 60 |
| steel_wire (lathe / 车床) | 4 s | 80 |
| Electrolyzer run / 电解一次 | 8 s | **800** |
| 1 coal / 1 块煤 | 80 s | injects 6,400 / 注入 6,400 |

**EN:** One coal powers ~8 full electrolysis runs (or 100+ lathe parts).
**中文：** 1 块煤约支持 8 次完整电解（或 100+ 个车床零件）。

---

## ⚠️ Troubleshooting / 常见问题

**EN:**
- *Stuck at PAUSED?* Check the GeneratorPanel (still burning?), then trace the 4-connected wire path to the machine's uppercase anchor — one gap breaks BFS; diagonals don't connect.
- *Generator burning but no power?* Wires must physically touch the generator or another conductive machine; the pool alone isn't enough — the anchor must be in `poweredMachines`.
- *Multiple machines per generator?* Fine — the pool is shared; only ensure total draw ≤ total injection over time.

**中文：**
- *一直 PAUSED？* 先看 GeneratorPanel（还在烧吗？），再检查到大写锚点的 4 邻接电线——断一格 BFS 即失败，对角不连通。
- *发电机在烧却没电？* 电线必须物理接触发电机或其他导电机器；只有 EU 池不够，锚点必须进入 `poweredMachines`。
- *一台发电机带多台机器？* 可以——池是共享的，只需长期总扣电 ≤ 总注入。
