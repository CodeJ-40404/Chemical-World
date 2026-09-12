# Machines / 机器

> **EN:** All machines except the Blast Furnace and Lathe (pre-placed at Home) are built via the Build menu (`B`) after buying a one-time blueprint. Powered machines need wire connection to a burning generator — see [Power System](Power-System.md).
> **中文：** 除土高炉与车床（家园预置）外，所有机器都需在 `T` 购买一次性蓝图后，通过 `B` 建造菜单放置。耗电机器必须用电线连到燃烧中的发电机——见[电力系统](Power-System.md)。

## Machine roster / 机器总览

| Char / 字符 | Machine / 机器 | Footprint / 占地 | BP / 蓝图 | Place / 放置 | Power / 耗电 |
|---|---|---|---|---|---|
| `F`/`f` | Blast Furnace 土高炉 | 2×2 | pre-built 预置 | — | none 无 |
| `L`/`l` | Lathe 车床 | 2×2 | pre-built 预置 | — | 2 EU/tick |
| `G`/`g` | Thermal Generator 火力发电机 | 2×2 | 150c | 100c | produces 8 EU/t 发电 |
| `+` | Wire 电线 | 1×1 | 50c | 5c | conductor 导体 |
| `X`/`x` | Crusher 破碎机 | 2×2 | 200c | 150c | 4 EU/active slot 每活跃槽 |
| `W`/`w` | Ore Washer 洗矿槽 | 2×2 | 160c | 120c | 2 EU/tick |
| `R`/`r` | Centrifuge 离心机 | 2×2 | 250c | 200c | 8 EU/tick |
| `S`/`s` | Gem Sorter 宝石筛选机 | 2×2 | 220c | 180c | 6 EU/tick |
| `Z`/`z` | **Electrolyzer 电解机** | 2×2 | **300c** | **250c** | **10 EU/tick** |
| `K`/`k` | **Chemistry Bench 化合台** | 2×2 | **180c** | **120c** | **none 无** |

> **EN / 中文：** Every 2×2 machine uses an uppercase anchor + three lowercase helper tiles; only the anchor is stored in `machineMeta` and matched by `E`.
> 所有 2×2 机器使用大写锚点 + 三个小写辅助格；只有锚点写入 `machineMeta` 并被 `E` 键匹配。

---

## 🔥 Blast Furnace / 土高炉（F）

**EN:** Pre-placed at Home (5,5). 4 parallel slots, 30 s per batch, 24-frame ASCII animation (STOKING → HEATING → POURING), **no power required**.

**中文：** 预置在家园 (5,5)。4 槽并行，每批 30 秒，24 帧 ASCII 动画（STOKING 铲煤 → HEATING 烧炼 → POURING 倾倒），**无需电力**。

| Property / 属性 | Value / 数值 |
|------|------|
| Slots / 槽数 | 4 parallel / 并行 |
| Time / 每批 | 30 s (30,000 ms) |
| Recipe cost / 配方消耗 | 2 ore + 1 coal (consumed inside `update`) / 2 矿石 + 1 煤 |
| Chars / 字符 | `F` bright red 亮红 / `f` dark red 暗红 |

### Recipes / 配方（8）

| Recipe / 配方 | Input ×2 / 输入 | Output / 输出 |
|--------|--------|------|
| Steel Making | hematite / magnetite | steel ×1 |
| Aluminum Smelt | bauxite | aluminum ×1 |
| Tin Smelting | cassiterite | tin ×1 |
| Copper Smelt | malachite / chalcopyrite | copper ×1 |
| Gold Smelting | gold_ore | gold_ingot ×1 |
| Silver Smelt | silver_ore | silver_ingot ×1 |

**EN:** UI (`E` nearby): slot radiobox → recipe menu → LOAD SLOT → auto smelt → COLLECT; CANCEL SLOT aborts. 4 slots run independently.
**中文：** 机器旁按 `E`：槽 Radiobox → 配方 Menu → LOAD SLOT → 自动烧炼 → COLLECT；CANCEL SLOT 中止。4 槽独立运行。

---

## ⚙️ Lathe / 车床（L）

**EN:** Pre-placed at Home (8,5). Machines 6 steel-part molds; draws 2 EU/tick while Inserting/Machining and auto-pauses (progress retained) without power.

**中文：** 预置在家园 (8,5)。6 种钢零件模具；Inserting/Machining 状态每 tick 扣 2 EU，无电自动暂停（保留进度）。

| Mold / 模具 | Input / 输入 | Output / 输出 | Time / 耗时 | XP | Sell / 售价 |
|------|------|------|------|------|------|
| Gear | steel ×1 | steel_gear ×1 | 3.0s | +12 | 25c |
| Rod | steel ×1 | steel_rod ×1 | 2.5s | +10 | 20c |
| Plate | steel ×1 | steel_plate ×1 | 2.0s | +8 | 18c |
| Spring | steel ×1 | steel_spring ×1 | 3.5s | +14 | 30c |
| Bolt | steel ×1 | steel_bolt ×2 | 1.5s | +6 | 12c |
| Wire | steel ×1 | steel_wire ×3 | 4.0s | +16 | 35c |

**State machine / 状态机：** `Idle → Inserting (5 frames) → Machining → Done → (COLLECT) → Idle`

---

## ⚡ Thermal Generator / 火力发电机（G）

| Property / 属性 | Value / 数值 |
|------|------|
| Cost / 成本 | BP 150c one-time + 100c per unit / 蓝图一次性 150c + 每台 100c |
| Per coal / 每煤 | 6,400 EU injected into the shared pool / 注入共享池 |
| Burn rate / 燃烧速度 | 8 EU/tick (100 ms) → 80 s per coal / 每煤 80 秒 |
| Chars / 字符 | `G` bright yellow / `g` dark yellow |

**EN:** `E` nearby opens GeneratorPanel → ADD 1 COAL (consumes 1 backpack coal, `burnEU += 6400`). Multiple burning generators stack injection; the pool caps at 10,000 EU.
**中文：** 旁按 `E` 打开 GeneratorPanel → ADD 1 COAL（消耗背包 1 煤，`burnEU += 6400`）。多台燃烧发电机注入叠加；EU 池上限 10,000。

---

## 🧱 Processing chain / 处理链（X → W → R）

**EN:** Player-built powered chain that upgrades raw ore. Each machine has its own recipe table, animation and per-slot/pause semantics modeled on the Lathe.

**中文：** 玩家自建的耗电处理链，用于升级原矿。每台机器有独立配方表、动画与每槽/暂停语义，模式同车床。

| Order / 顺序 | Machine / 机器 | Power / 耗电 | Role / 作用 |
|------|------|------|------|
| 1 | **Crusher `X`** (yellow / 黄) | 4 EU per active slot / 每活跃槽 | Ore → crushed ore + gravel / 矿石→碎矿+砾石 |
| 2 | **Ore Washer `W`** (cyan / 青) | 2 EU/tick | Crushed ore → purified ore + byproducts / 碎矿→净化矿+副产 |
| 3 | **Centrifuge `R`** (purple / 紫) | 8 EU/tick | Purified ore → dust (e.g. `bauxite_dust`, 22c) + byproducts / 净化矿→矿粉（如 `bauxite_dust`）+ 副产 |

**Branch / 支线：** **Gem Sorter `S`** (white/grey 白/灰, 6 EU/tick) sorts crushed gems by grade for "surprise output" gems.

> **EN / 中文：** `bauxite_dust` from the Centrifuge is the feedstock for the Electrolyzer's alumina recipe — the processing chain feeds directly into chemistry.
> 离心机产出的 `bauxite_dust`（铝土粉，22c）正是电解机铝土配方的原料——处理链直接对接化学系统。

---

## 🧪 Chemistry machines / 化学机器（Z, K） ⭐ v0.5.0

### Electrolyzer / 电解机（`Z`/`z`, cyan / 青色）

**EN:** 10 EU/tick (800 EU per 8 s run), BP 300c + 250c placement. Breaks compounds into elements with multi-product output; pauses unpowered. 4 recipes: water, alumina, molten salt, chlor-alkali.

**中文：** 10 EU/tick（8 秒/次，共 800 EU），蓝图 300c + 放置 250c。将化合物分解为元素，支持多产物；断电暂停。4 条配方：水、铝土、熔盐、氯碱。

### Chemistry Bench / 化合台（`K`/`k`, green / 绿色）

**EN:** No power, instant craft, BP 180c + 120c placement. 5 recipes recombine elements into water, salt, NaOH, CuSO₄, FeS; the UI shows live have/need per input.

**中文：** 不耗电、即时合成，蓝图 180c + 放置 120c。5 条配方将元素合成为水、盐、NaOH、CuSO₄、FeS；界面实时显示每种输入的 have/need。

➡️ **Full recipes, element tables and economy math: [Chemistry](Chemistry.md)**
➡️ **完整配方、元素表与经济计算：[化学系统](Chemistry.md)**

---

## 📐 2×2 anchor convention / 2×2 锚点约定

```
F f     L l     G g     X x     Z z     K k
F f     L l     G g     X x     Z z     K k
↑ anchor (top-left), stored in machineMeta / 锚点（左上），存入 machineMeta
```

- **EN:** Helpers are display-only; `isNear(...,'F',...)` matches the uppercase anchor only; wires `+` are 1×1 with no anchor.
- **中文：** 辅助格仅用于显示；`isNear(...,'F',...)` 只匹配大写锚点；电线 `+` 为 1×1，无锚点。
- **EN:** Default new-game layout: `F` (5,5), `L` (8,5); everything else is player-built.
- **中文：** 新游戏默认布局：`F` (5,5)、`L` (8,5)；其余全部由玩家建造。

---

## FAQ / 常见问题

**EN:**
- *Machine shows PAUSED?* Generator out of coal or wire path broken — BFS needs 4-connected tiles all the way to the anchor.
- *Can machines be removed?* Not yet — placement is permanent.
- *Do machines save?* Machine state is rebuilt from `machineMeta` + tile rehydration on load; EU/blueprints persist in the v7 save.

**中文：**
- *机器显示 PAUSED？* 发电机缺煤或电线断了——BFS 需要一路 4 邻接通到锚点。
- *机器能拆吗？* 暂不支持，放置即永久。
- *机器会存档吗？* 加载时由 `machineMeta` + 瓦片 rehydration 重建；EU/蓝图在 v7 存档中持久化。
