# Controls / 按键操作

> **EN:** Complete key bindings and FTXUI UI navigation.
> **中文：** 完整按键列表与 FTXUI 界面操作技巧。

---

## 🎮 World map / 大地图按键

| Key / 按键 | EN / 中文 |
|------|------|
| `W` `A` `S` `D` / arrows 方向键 | Move / 移动 |
| `E` | Context interact (see table below) / 上下文交互（见下表） |
| `B` | Build mode (Home only) / 建造模式（仅家园） |
| `C` | Backpack / 背包 |
| `T` | Trading market SELL/BUY / 交易市场 |
| `K` | Bank & investment desk / 银行与投资台 |
| `J` | Quest book / 任务书 |
| `F` | Furnace location hint / 高炉位置提示 |
| `H` | Help / 帮助 |
| `P` / `L` | Save / load current manual slot / 保存 / 读取当前手动档 |
| `Q` | Quit with autosave / 退出并自动保存 |
| `SPACE` | Track / untrack quest (in quest book) / 追踪/取消追踪任务 |
| `R` | Claim quest reward / 领取任务奖励 |
| `+` `-` | Zoom quest book / 缩放任务书 |

> **EN / 中文：** The Chemistry Bench's **map character** is `K`, but it opens with **`E`** (like every machine); the **`K` key** opens the bank.
> 化合台的**地图字符**是 `K`，但用 **`E` 键**打开（与所有机器一致）；**`K` 键**打开的是银行。

---

## 🔀 E-key context dispatch / E 键上下文分发

**EN:** Checks the 4 orthogonally adjacent tiles (no diagonals) in priority order:
**中文：** 按优先级检查上下左右 4 邻接（不含对角）：

| Priority / 优先级 | Nearby / 附近 | Action / 行为 |
|--------|----------|------|
| 1 | Furnace `F` | FurnaceUI |
| 2 | Lathe `L` | LatheUI |
| 3 | Generator `G` | GeneratorPanel |
| 4 | Crusher `X` | CrusherUI |
| 5 | Washer `W` | WasherUI |
| 6 | Centrifuge `R` | CentrifugeUI |
| 7 | Gem Sorter `S` | SorterUI |
| 8 | **Electrolyzer `Z`** | **ElectrolyzerUI** |
| 9 | **Chemistry Bench `K`** | **ChemBenchUI** |
| 10 | Car `C` (Home/Wasteland) | Travel Home ↔ Wasteland / 家园↔荒原 |
| 11 | Cave entrance/exit `O` | Enter / leave cave / 进出矿洞 |
| 12 | Ore tile / 矿物 | Mine one hit (3 per ore) / 敲击一次 |
| — | none 无 | "Nothing nearby to interact with." |

> **EN / 中文：** Only uppercase anchors match (`isNear(...,'Z',...)`); lowercase helper tiles never trigger.
> 只有大写锚点会匹配，小写辅助格永不触发。

---

## 🖥️ FTXUI fullscreen UIs / 全屏界面通用操作

| Key / 按键 | EN / 中文 |
|------|------|
| `Tab` | Move focus between menu / radiobox / buttons / 在组件间切换焦点 |
| `↑` `↓` | Select in Menu / Radiobox / 在菜单中上下选择 |
| `←` `→` | Switch radiobox tabs (e.g. SELL/BUY) / 左右切换标签 |
| `Enter` | Activate focused button / 触发聚焦按钮 |
| `ESC` | Close the current screen / 关闭当前界面 |

---

## 🧪 Chemistry UIs (v0.5.0) / 化学界面

### Electrolyzer UI / 电解机界面

| Control / 控件 | EN / 中文 |
|---|---|
| Recipe radiobox / 配方 Radiobox | 4 recipes, ↑/↓ to select / 4 条配方上下选 |
| `LOAD` | Consume inputs, start 8 s run (10 EU/tick) / 消耗输入开始 8 秒加工 |
| `COLLECT` | Take all outputs (enabled when Done) / 取走全部产物（完成后可用） |
| progress bar / 进度条 | Live 100 ms ticker; greyed = PAUSED (no power) / 实时刷新；灰色=断电暂停 |
| `CLOSE` / `ESC` | Exit / 退出 |

### Chemistry Bench UI / 化合台界面

| Control / 控件 | EN / 中文 |
|---|---|
| Recipe menu / 配方菜单 | 5 syntheses with per-input `have/need` (green/red) / 5 条合成，输入显示 have/need 红绿 |
| `CRAFT` | Instant craft; rejected if any input missing / 即时合成，缺料被拒 |
| `CLOSE` / `ESC` | Exit (no ticker — machine uses no power) / 退出（无 ticker——不耗电） |

### Other machine UIs / 其他机器界面

| UI | Buttons / 按钮 |
|---|---|
| Furnace | LOAD SLOT · CANCEL SLOT · COLLECT · CLOSE (4-slot radiobox + 8 recipes) |
| Lathe | LOAD/SELECT · CANCEL · COLLECT · CLOSE (6 molds) |
| Generator | ADD 1 COAL · CLOSE |
| Crusher/Washer/Centrifuge/Sorter | Per-slot load/collect pattern + CLOSE, same Tab/Enter/ESC model |

---

## 🏗️ Build / Trade / Bank / Quest UIs

| UI / 界面 | Keys / 按键 |
|---|---|
| Build Mode `B` | ↑/↓ select 10 placeables · Enter/PLACE at center tile · ESC / 上下选 10 种可放置物，Enter 放置 |
| Market `T` | ↑/↓ on tab radiobox to switch SELL/BUY · SELL ONE/ALL/EVERYTHING · BUY 1 / 切标签、买卖 |
| Bank `K` | Buy/sell Gold & Bitcoin; prices fluctuate / 买卖黄金与比特币 |
| Quest Book `J` | ↑/↓ browse · SPACE track · R claim · +/- zoom / 浏览、追踪、领奖、缩放 |

---

## 🎬 Startup & exit / 启动与退出

| Key / 按键 | When / 时机 | EN / 中文 |
|---|---|---|
| `1` `2` `3` | startup / 启动 | Choose manual slot 1–3 / 选择手动档 |
| `Enter` | startup / 启动 | Confirm / 确认 |
| `Q` | in game / 游戏中 | Autosave to slot 0 + quit / 自动存档并退出 |

---

## 🐛 Historical notes / 历史问题

**EN / 中文：**
- Old help text once listed `F5 Save / F9 Load`, which were dead cases — removed; use `P` / `L`. / 旧帮助曾写 F5/F9，实为死代码，已删除。
- Early TradeUI locked focus on SELL because both menus lacked Maybe guards — fixed. / 早期 TradeUI 焦点锁死在 SELL（缺 Maybe 门控），已修复。
