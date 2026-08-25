# Controls

完整按键列表与 FTXUI 操作技巧。

---

## 🎮 游戏主循环按键

| 按键 | 功能 | 备注 |
|------|------|------|
| `W` `A` `S` `D` | 上下左右移动 | 方向键 ↑↓←→ 同效 |
| `E` | 交互（上下文敏感） | 见下表 |
| `B` | 打开建造模式 | 仅家园 |
| `C` | 打开合成菜单 | |
| `T` | 打开交易市场 | |
| `F` | 高炉位置提示 | 仅提示，不打开 UI |
| `H` | 显示帮助 | |
| `P` | 保存到当前手动档 | slot 1/2/3 |
| `L` | 读取当前手动档 | slot 1/2/3 |
| `Q` | 退出游戏 | 自动保存到 autosave |

> ⚠️ 早期版本 help 写 `F5 Save / F9 Load`，**实际无效**（死代码）。已修正为 `P / L`。

---

## 🔀 E 键上下文交互

`E` 会按以下顺序检查周围 4 邻接（上下左右）：

| 优先级 | 触发条件 | 行为 |
|--------|----------|------|
| 1 | 家园 + 高炉 `F` 锚点旁 | 打开 FurnaceUI |
| 2 | 家园 + 车床 `L` 锚点旁 | 打开 LatheUI |
| 3 | 家园 / 荒原 + Car `C` 旁 | 切换家园 ↔ 荒原 |
| 4 | 荒原 + 矿洞入口 `O` 旁 | 进入矿洞 |
| 5 | 矿洞 + 矿洞出口 `O` 旁 | 离开矿洞 |
| 6 | 家园 + 任何发电机 `G` 锚点旁 | 打开 GeneratorPanel |
| 7 | 任何 + 矿物 tile 旁 | 开采（敲 3 下出矿） |
| - | 都不匹配 | "Nothing nearby to interact with." |

> 💡 `isNear(x, y, 'F', px, py)` 只检查大写锚点字母，不会误触发辅助格 `f`。

---

## 🖥️ FTXUI 全屏界面操作

所有 FTXUI 全屏界面（FurnaceUI / LatheUI / TradeUI / BuildUI / GeneratorPanel）通用操作：

| 按键 | 行为 |
|------|------|
| `Tab` | 在组件间切换焦点（菜单 / 按钮 / Radiobox） |
| `↑` `↓` | 在 Menu / Radiobox 中上下选择 |
| `←` `→` | 在 Radiobox 中左右切换（如 SELL/BUY Tab） |
| `Enter` | 确认 / 触发当前按钮 |
| `ESC` | 关闭当前界面 |

> ⚠️ 不同界面的 ESC 行为可能不同，详见各界面 wiki 页。

---

## 🔥 FurnaceUI（土高炉）

| 按键 | 行为 |
|------|------|
| `Tab` | 在 槽Radiobox / 配方Menu / 按钮间切换 |
| `↑` `↓` | 在 4 槽 Radiobox 中选 Slot 0-3 / 在配方 Menu 中选 8 种配方 |
| `Enter` | 触发当前聚焦的按钮 |
| `ESC` | 关闭 FurnaceUI |

### 按钮

| 按钮 | 行为 |
|------|------|
| `LOAD SLOT` | 把当前选中配方装载到当前槽，开始 30s 烧炼 |
| `CANCEL SLOT` | 中止当前槽的烧炼 |
| `COLLECT` | 取走当前槽的产物 |
| `CLOSE` | 关闭界面 |

---

## ⚙️ LatheUI（车床）

| 按键 | 行为 |
|------|------|
| `Tab` | 在 模具Menu / 按钮间切换 |
| `↑` `↓` | 在 6 种模具中选择 |
| `Enter` | 触发当前聚焦的按钮 |
| `ESC` | 关闭 LatheUI |

### 按钮

| 按钮 | 行为 | 显示条件 |
|------|------|----------|
| `LOAD` | 装载选中模具，消耗 1 steel | Idle 状态 |
| `SELECT` | 同 LOAD（确认选择） | Idle 状态 |
| `CANCEL` | 中止加工 | Machining 状态 |
| `COLLECT` | 取走产物 | Done 状态 |
| `CLOSE` | 关闭界面 | 任何状态 |

---

## 🏪 TradeUI（交易市场）

| 按键 | 行为 |
|------|------|
| `Tab` | 在 Tab Radiobox / Menu / 按钮间切换 |
| `↑` `↓` | 切 SELL/BUY Tab / 选条目 |
| `Enter` | 触发按钮 |
| `ESC` | 关闭 TradeUI |

### SELL Tab 按钮

| 按钮 | 行为 |
|------|------|
| `SELL ONE` | 卖出选中物品 ×1 |
| `SELL ALL` | 卖出选中物品全部 |
| `SELL EVERYTHING` | 清空所有可售物品 |
| `CLOSE` | 关闭界面 |

### BUY Tab 按钮

| 按钮 | 行为 |
|------|------|
| `BUY 1` | 买入选中物品 ×1 / 解锁蓝图 |
| `CLOSE` | 关闭界面 |

---

## 🏗️ BuildUI（建造模式）

| 按键 | 行为 |
|------|------|
| `Tab` | 在 placeables Menu / 按钮间切换 |
| `↑` `↓` | 在 4 种可放置物中选择 |
| `Enter` | 触发按钮 |
| `ESC` | 关闭 BuildUI |

### 按钮

| 按钮 | 行为 |
|------|------|
| `PLACE` | 在玩家脚下（网格中心）放置选中物 |
| `CLOSE` | 关闭界面 |

---

## ⚡ GeneratorPanel（发电机面板）

| 按键 | 行为 |
|------|------|
| `Enter` | 触发 `ADD 1 COAL` 或 `CLOSE` |
| `ESC` | 关闭面板 |

### 按钮

| 按钮 | 行为 |
|------|------|
| `ADD 1 COAL` | 消耗背包 1 个 coal，发电机 burnEU += 6400 |
| `CLOSE` | 关闭面板 |

---

## 🎬 启动 / 退出

| 按键 | 时机 | 行为 |
|------|------|------|
| `1` `2` `3` | 启动时存档选择 | 选 slot 1/2/3 加载 |
| `Enter` | 启动时 | 确认选择 |
| `Q` | 游戏中 | 退出 + autosave |
| 任意键 | 退出确认页 | 关闭游戏 |

---

## 🐛 已知问题与历史

### F5/F9 死代码（已修复）

早期版本 help 文本写 `F5 Save / F9 Load`，但代码中：
```cpp
case 63: processKey(1005); break;  // F5
case 67: processKey(1009); break;  // F9
```

`processKey` 的 switch 里**没有 case 1005/1009**，落到 `default: return;` —— 按了没反应。

实际可用的是 `P`（保存）/ `L`（读取）。已删除 F5/F9 死代码并修正 help 文本。

### BUY Tab 选不了（已修复）

早期 TradeUI 的 menuSell / menuBuy 都塞进 Container 没有 Maybe 门控，焦点锁在 menuSell。已修复用 Maybe 门控让非当前 tab 菜单失活。
