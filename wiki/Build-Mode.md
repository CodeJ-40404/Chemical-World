# Build Mode / 建造模式

> **EN:** Press `B` at Home to open the 7×7 zoomed build grid. Home only; other areas show "Can only build at home."
> **中文：** 在家园按 `B` 打开 7×7 放大建造网格。仅限家园；其他区域提示 "Can only build at home."

**Entry / 入口：** `B` (Home only) · **Exit / 退出：** `ESC` or CLOSE

---

## 🖥️ Layout / 界面

**EN:** Left = placeables menu; right = coins, live EU pool, blueprint ownership; bottom = 7×7 preview (center tile has a DOUBLE border = your position = placement target); PLACE / CLOSE buttons.
**中文：** 左侧=可放置物菜单；右侧=金币、实时 EU 池、蓝图持有状态；底部=7×7 预览（中心格双线框=玩家位置=放置目标）；PLACE / CLOSE 按钮。

---

## 📐 7×7 grid / 7×7 网格

| EN | 中文 |
|----|------|
| Area: dx, dy ∈ [-3, +3] around the player | 范围：以玩家为中心 dx, dy ∈ [-3, +3] |
| Center tile = DOUBLE border = placement cell | 中心格双线框=放置位置 |
| Other tiles = LIGHT border, colored by display char | 其他格单线框，按字符着色 |

### Color map / 颜色映射

| Char / 字符 | Meaning / 含义 | Color / 颜色 |
|------|------|------|
| `.` | grass 草地 | bright green 亮绿 |
| `~` | river/lake 河流湖泊 | blue / dark cyan 蓝/深青 |
| `*` `v` | flower / grass tuft 花/草丛 | purple / green 紫/绿 |
| `+` | wire 电线 | blue/yellow 蓝/黄 |
| `G`/`g` | Generator 发电机 | yellow / dark yellow 黄/暗黄 |
| `F`/`f` | Furnace 高炉 | red / dark red 红/暗红 |
| `L`/`l` | Lathe 车床 | purple / dark purple 紫/暗紫 |
| `X`/`x` | Crusher 破碎机 | yellow / dark yellow |
| `W`/`w` | Washer 洗矿槽 | cyan / dark cyan 青/暗青 |
| `R`/`r` | Centrifuge 离心机 | purple / dark purple |
| `S`/`s` | Gem Sorter 筛选机 | white / grey 白/灰 |
| `Z`/`z` | **Electrolyzer 电解机** | **cyan / dark cyan 青/暗青** |
| `K`/`k` | **Chemistry Bench 化合台** | **green / dark green 绿/暗绿** |

---

## 🛠️ Placeables / 可放置物（10）

| Idx / 序号 | Name / 名称 | Size / 尺寸 | Cost / 放置费 | Blueprint / 蓝图 | Char / 字符 |
|------|------|------|------|----------|------|
| 0 | Generator 发电机 | 2×2 | 100c | Generator BP 150c | `G`/`g` |
| 1 | Wire 电线 | 1×1 | 5c | Wire BP 50c | `+` |
| 2 | Crusher 破碎机 | 2×2 | 150c | Crusher BP 200c | `X`/`x` |
| 3 | Ore Washer 洗矿槽 | 2×2 | 120c | Washer BP 160c | `W`/`w` |
| 4 | Centrifuge 离心机 | 2×2 | 200c | Centrifuge BP 250c | `R`/`r` |
| 5 | Gem Sorter 宝石筛选机 | 2×2 | 180c | Sorter BP 220c | `S`/`s` |
| 6 | **Electrolyzer 电解机** | 2×2 | **250c** | **Electrolyzer BP 300c** | `Z`/`z` |
| 7 | **Chemistry Bench 化合台** | 2×2 | **120c** | **Chem Bench BP 180c** | `K`/`k` |
| 8 | Flower 花 | 1×1 | 1c | none 无 | `*` |
| 9 | Grass tuft 草丛 | 1×1 | 1c | none 无 | `v` |

> **EN / 中文：** Menu order and the `tryPlace` case indices are kept in lock-step (0–9). Locked entries show `[LOCKED: buy BP first]`.
> 菜单顺序与 `tryPlace` 的 case 序号严格一致（0–9）。未解锁显示 `[LOCKED: buy BP first]`。

### Placement rules / 放置规则

**EN:**
- **2×2 machines:** all 4 tiles must be empty grass (`.`); costs coins; stamps anchor + 3 helpers, sets all impassable, pushes `machineMeta`, creates the runtime instance.
- **Wire:** target tile must be passable; stamps `+`, impassable.
- **Flower / grass:** 1c; stamped decor **stays passable** (you can walk on it).
- Machines cannot be removed after placement.

**中文：**
- **2×2 机器：** 4 格必须全是空草地（`.`）；扣币；盖印锚点+3 辅助格、全部设为不可通行、写入 `machineMeta` 并创建运行时实例。
- **电线：** 目标格须可通行；盖印 `+`，不可通行。
- **花 / 草丛：** 1c；装饰**保持可通行**（可踩踏）。
- 机器放置后不可拆除。

---

## ⚡ 100 ms ticker / 100ms 心跳

**EN:** BuildUI runs a background ticker, so the EU pool and machine progress update live while the menu is open.
**中文：** BuildUI 运行后台 ticker，菜单打开期间 EU 池与机器进度实时更新。

---

## 📋 Worked example / 完整示例

**EN:** Place a generator and power the Lathe:
**中文：** 放发电机并给车床通电：

```
1. T → BUY → Generator Blueprint (150c), Wire Blueprint (50c)
2. B → select Generator → PLACE at (15,5)   (stamps G/g, -100c)
3. Walk toward (8,5), PLACE wire on each tile (5c each)
4. ESC → E on the generator → ADD 1 COAL
5. E on the Lathe → machining now runs
```

**EN (v0.5.0):** Repeat with Electrolyzer BP (300c) + Chem Bench BP (180c); the Electrolyzer needs the same wire treatment, the bench does not.
**中文（v0.5.0）：** 电解机蓝图（300c）与化合台蓝图（180c）同理；电解机需要接线，化合台不需要。

---

## ⚠️ FAQ / 常见问题

**EN / 中文：**
- *Cannot place a 2×2?* All four cells must be empty `.` — check the preview. / 4 格必须全是空 `.`，查看预览。
- *Can wires overlap the player?* Placement resolves at your current tile; move first. / 按当前脚下格放置，请先移动。
- *Do decor tiles conduct?* No — only `+` and machine chars are conductors. / 不导电，只有 `+` 与机器字符导电。
