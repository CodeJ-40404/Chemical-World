# Save System / 存档系统

> **EN:** Custom text/binary hybrid format, current version **v7**; the loader accepts v2–v7 and always upgrades on next save.
> **中文：** 自定义文本/二进制混合格式，当前版本 **v7**；加载器接受 v2–v7，下次保存时自动升级。

---

## 📁 Files / 存档文件

| File / 文件 | Slot / 档位 | Written / 写入时机 |
|------|------|----------|
| `chemical_world_slot1.sav` | manual 1 / 手动 1 | `P` with activeSaveSlot=1 |
| `chemical_world_slot2.sav` | manual 2 / 手动 2 | `P` with activeSaveSlot=2 |
| `chemical_world_slot3.sav` | manual 3 / 手动 3 | `P` with activeSaveSlot=3 |
| `chemical_world_autosave.sav` | slot 0 / 自动档 | `Q` quit with autosave / 退出自动保存 |

**EN:** Path = the executable's working directory.
**中文：** 路径 = 可执行文件所在工作目录。

---

## 📝 v7 format / v7 格式

**Header / 文件头：** `CHEMICAL_WORLD_SAVE 7`

```
CHEMICAL_WORLD_SAVE 7
<player.name>
<level> <coins> <exp> <x> <y>
<currentArea int>                    # 0=Home 1=Wasteland 2=Cave
<inventory.size()>
<item.name> <qty> <category> <value> # one line per item / 每行一个物品
<tutorial flags ...>
<width> <height>                     # GameMap
<tile rows ...>                      # display/name/interact/passable/color
<8 blueprint ints 0/1>               # gen wire crusher washer centrifuge sorter electrolyzer chembench
<globalEU>
<machineMeta.size()>
<x> <y> <type> <remainingBurnEU> <loadedCoal> <active>   # per machine / 每行一台机器
```

### Blueprint flags / 蓝图标记（8）

| Order / 顺序 | Flag / 标记 | Introduced / 引入版本 |
|---|---|---|
| 1 | `gen_blueprint_unlocked` | v3 |
| 2 | `wire_blueprint_unlocked` | v3 |
| 3 | `crusher_blueprint_unlocked` | v4 |
| 4 | `washer_blueprint_unlocked` | v4 |
| 5 | `centrifuge_blueprint_unlocked` | v4 |
| 6 | `sorter_blueprint_unlocked` | v6 |
| 7 | **`electrolyzer_blueprint_unlocked`** | **v7** |
| 8 | **`chembench_blueprint_unlocked`** | **v7** |

### Inventory categories / 背包分类

`ore` · `basic` · `fuel` · `material` · `product` · **`element`** · **`compound`** · `gem` · `blueprint` · `misc`

### Machine types / 机器类型

`F` furnace · `L` lathe · `G` generator · `X` crusher · `W` washer · `R` centrifuge · `S` sorter · **`Z` electrolyzer** · **`K` chemistry bench**

---

## 🔄 Backward compatibility / 向后兼容

**EN:** `loadGame` accepts versions 2–7. Missing trailing fields get explicit defaults:

**中文：** `loadGame` 接受 v2–v7。缺失的尾部字段取显式默认值：

| Loading / 加载 | Behavior / 行为 |
|---|---|
| v2 | blueprints default false, EU=0; anchors reconstructed by scanning map tiles / 蓝图默认 false、EU=0，扫描地图重建锚点 |
| v3–v5 | read only the flags that existed then; newer BP flags = false / 仅读当时存在的标记，新蓝图=false |
| v6 | 6 blueprint ints; `electrolyzer`/`chembench` default false, trailing ints **not read** / 读 6 个蓝图，新两者 false，不读末尾 |
| v7 | full 8-int blueprint line / 完整 8 个蓝图 |

> **EN / 中文：** Loading a v2–v6 save then pressing `P` rewrites it as v7 — no data is lost and the older game can't read the upgraded file (by design).
> 加载 v2–v6 旧档后按 `P` 即重写为 v7——不丢数据；旧版游戏无法读取升级后的文件（设计如此）。

---

## 🏗️ Machine rehydration / 机器重建

**EN:** Runtime machine instances (counters, animation frames, loaded recipes) are **not serialized**. On load:
1. `machineMeta` entries provide type + anchor coordinates.
2. `rehydrateMachineTiles()` re-stamps the 2×2 display tiles.
3. Containers (`generators`, `crushers`, `washers`, `centrifuges`, `sorters`, `electrolyzers`, `chemBenches`) lazily find-or-insert default instances keyed by anchor.

**中文：** 运行时机器实例（计数器、动画帧、已装配方）**不序列化**。加载时：
1. `machineMeta` 提供类型 + 锚点坐标；
2. `rehydrateMachineTiles()` 重新盖印 2×2 显示瓦片；
3. 各容器（`generators`/`crushers`/.../`electrolyzers`/`chemBenches`）以锚点为键 find-or-insert 默认实例。

**EN:** Persisted per machine: type, anchor, and for generators `remainingBurnEU` / `loadedCoal` / `active`.
**中文：** 每台机器持久化：类型、锚点；发电机另有 `remainingBurnEU` / `loadedCoal` / `active`。

---

## 🎮 Controls / 存档操作

| Key / 按键 | EN / 中文 |
|---|---|
| startup `1`/`2`/`3` + Enter | choose a slot; failed load → new game / 选档；读取失败→新游戏 |
| `P` | save to active manual slot / 保存到当前手动档 |
| `L` | load active manual slot / 读取当前手动档 |
| `Q` | autosave to slot 0 then quit / 自动存档后退出 |

---

## ⚠️ FAQ / 常见问题

**EN:**
- *Will loading an old save break?* No — v2+ loads; new blueprints start locked.
- *Does autosave overwrite a manual slot?* No — slot 0 vs 1/2/3.
- *Do item names use spaces/unicode?* No — ASCII identifiers (`sodium_hydroxide`), safe for the space-delimited format.
- *Can I delete saves?* Only by deleting the `.sav` file; no in-game deletion.

**中文：**
- *旧档会坏吗？* 不会——v2+ 均可加载，新蓝图默认未解锁。
- *autosave 会覆盖手动档吗？* 不会——slot 0 与 1/2/3 分离。
- *物品名有空格/中文吗？* 没有——ASCII 标识符（如 `sodium_hydroxide`），空格分隔格式安全。
- *能删档吗？* 只能删除 `.sav` 文件，游戏内无删除功能。
