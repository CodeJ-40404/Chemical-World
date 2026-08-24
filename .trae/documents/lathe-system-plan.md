# Lathe（车床）系统实现计划

## Context

游戏 [Chemical-World.cpp](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp) 已有 `BlastFurnace` + `FurnaceUI` 这套“地图固定设施 + FTXUI 全屏界面 + 100ms `Event::Custom` ticker 驱动”的完整范例。用户希望在 Home 地图新增一座车床，让玩家把 `steel` 加工成齿轮/杆/板等机械件，并配 ASCII 动画。

**已确认的设计决策（用户选定）：**
1. **形式**：地图固定设施（Home 区放 `'L'` 图块，按 E 邻近交互打开），完全镜像 furnace 模式，零存档改动。
2. **模具**：扩展集 6-8 种（gear/rod/plate/spring/bolt/wire，可选 shaft/ring），不同模具消耗/耗时/价值不同。
3. **动画**：FTXUI 内嵌（`Renderer` 每帧重画 ASCII，由现有 100ms ticker 推进帧索引），不切控制台、不闪烁。

**预期结果**：新游戏默认在 Home 有一座车床；玩家走近按 E 进入界面；选模具时 artBox 显示对应 ASCII 图标；按 LOAD 播放 5 帧“钢料滑入卡盘”动画；加工中循环播放 4 帧“车削/铁屑/火花”动画并推进进度条；完成后产出物品入库并加经验。

---

## 新类型

### `Mold` 结构体（Lathe 内嵌套，仿 [BlastFurnace::Recipe](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L129-L137)）

```cpp
struct Mold {
    string name;            // "Gear" — Radiobox 显示
    string result;          // "gear" — addItem 名字
    int steelRequired;      // 消耗 steel 数
    int resultAmount;       // 产出数
    int durationMs;         // 加工耗时（进度条满所需）
    int exp;                // 完成经验
    int value;              // product 类别价值（trade 用）
    vector<string> iconArt; // 静态 ASCII 图标（≤24 列 × ≤10 行，纯基础 ASCII）
};
```

### molds 表

| Mold | result | steel | amount | durationMs | exp | value |
|---|---|---|---|---|---|---|
| Gear | gear | 1 | 1 | 3000 | 12 | 25 |
| Rod | rod | 1 | 1 | 2500 | 10 | 20 |
| Plate | plate | 1 | 1 | 2000 | 8 | 18 |
| Spring | spring | 1 | 1 | 3500 | 14 | 30 |
| Bolt | bolt | 1 | 2 | 1500 | 6 | 12 |
| Wire | wire | 1 | 3 | 4000 | 16 | 35 |
| Shaft（可选） | shaft | 2 | 1 | 5000 | 20 | 45 |
| Ring（可选） | ring | 1 | 1 | 3000 | 12 | 25 |

价值对齐高炉 product value=20（[L291](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L291)），按复杂度微调。

### `Lathe` 类（仿 [BlastFurnace](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L112-L330)，去温度、加动画状态）

**插入位置：[L331](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L331)**（BlastFurnace `};` 之后，地图系统之前）

私有状态：`isRunning`、`loaded`、`progress`、`maxProgress`、`elapsedMs`、`selectedMold`、`loadedMold`、`frameIndex`、`enum AnimState { Idle, Inserting, Machining, Done } animState`、`logs`、`molds`、`insertFrames`（5 帧）、`machiningFrames`（4 帧循环）。

公共方法（签名）：
```cpp
Lathe();                                         // ctor 填充 insertFrames/machiningFrames
void addLog(const string&);
string getStatus();                              // "free"/"inserting"/"machining"/"done"
int  getProgress(); int getMaxProgress();
bool isActive(); bool isLoaded();
int  getSelectedMold(); AnimState getAnimState();
void selectMold(int);                            // 仅 Idle 可改选
bool canLoad(PlayerData&);                       // !isRunning && hasItem("steel", steelRequired)
void loadMaterials(PlayerData&);                 // 扣 steel、animState=Inserting、frameIndex=0
bool update(PlayerData&, int elapsedMs=100);     // 状态机+帧推进+完成奖励（见下）
void cancel();                                   // 仅 Machining 可取消，回 Idle（不退 steel，仿 L309）
void collect();                                  // Done→Idle
vector<string> getLogs();
vector<string> getMoldList();                     // Radiobox 列表
string getMoldInfo();                             // "Gear: 1 steel -> 1 gear, 3.0s"
vector<string> getMoldArt(int);                  // 选中模具静态 icon
vector<string> getCurrentFrame();                // 按 animState 取当前动画帧
```

**`update()` 状态机**（替换高炉温度逻辑；一次调用同时推进进度与动画帧 —— 这是动画机制的核心）：
- `Inserting`：`frameIndex++`；达 5 帧（500ms）→ 转 `Machining`，`isRunning=loaded=true`，`progress=elapsedMs=frameIndex=0`。
- `Machining`：`frameIndex++`、`elapsedMs+=elapsedMs`、`progress=min(max, elapsedMs*max/durationMs)`；`elapsedMs>=durationMs` 时 → `player.addItem(result, amount, "product", value)`（仿 [L291](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L291)）、`player.exp+=mold.exp`、升级检查（仿 [L296-302](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L296-L302)）、`animState=Done`、返回 true。
- `Idle`/`Done`：不推进。

---

## LatheUI（FTXUI 界面）

**插入位置：[L795](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L795)**（FurnaceUI `};` 之后）

### 组件成员（仿 [FurnaceUI L515-530](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L515-L530)）
`mainContainer`、`moldList`(Radiobox)、`buttonLoad`、`buttonCancel`、`buttonCollect`、`buttonClose`、`progressBar`、`logViewer`、`artBox`（新：渲染 ASCII 动画/icon）、`moldNames`。

### 视图状态机 + Maybe 门控（复用 [L654-668](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L654-L668) 的 Maybe idiom）
四个 `show*` lambda 基于 `lathe.getAnimState()`。`Container::Vertical` layout 用 `Maybe(button, showFn)` 门控每个按钮可见/可聚焦，避免 Tab 误触不可见按钮（这正是 put-button bug 的教训）：
```cpp
Maybe(moldList,     showIdle)
Maybe(buttonLoad,   showIdle)
Maybe(buttonCancel, showMachining)
Maybe(buttonCollect,showDone)
Maybe(buttonClose,  []{return true;})   // 全状态可关
```

### artBox 渲染器（新；仿 [logViewer L637-648](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L637-L648)）
```cpp
artBox = Renderer([&] {
    vector<string> frame = (lathe.getAnimState()==Lathe::Idle)
        ? lathe.getMoldArt(lathe.getSelectedMold())
        : lathe.getCurrentFrame();
    Elements lines; for (auto& l : frame) lines.push_back(text(l));
    return vbox(lines) | border | size(WIDTH, GREATER_THAN, 28) | size(HEIGHT, GREATER_THAN, 12);
});
```
不用 `Canvas`（全文件无 Canvas 用法，ASCII 都用 `text()`+`vbox`，见 [L933-957](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L933) titleArt）。

### mainContainer Renderer 分支（仿 [L677-769](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L677-L769)）
按 `animState` 分四支：Idle（主视图：状态+mold info+steel 库存+artBox+LOAD/CLOSE+日志）、Inserting（标题+artBox+状态消息）、Machining（标题+artBox+进度条+CANCEL+日志）、Done（标题+artBox+结果行+COLLECT/CLOSE）。

### CatchEvent 钩子（仿 [L771-780](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L771-L780)）—— 动画与进度共用
```cpp
mainContainer |= CatchEvent([&](Event event) {
    if (event == Event::Custom) {
        if (lathe.update(player)) statusMessage = "Machining complete! Press COLLECT.";
        needRefresh = true;
        return true;
    }
    return false;
});
```
ticker 每 100ms `PostEvent(Event::Custom)` → CatchEvent 调一次 `lathe.update()` → 状态机与帧索引同步推进 → 下一帧 UI 自动刷新。

### moldList 的 CatchEvent（仿 [L555-561](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L555-L561)）
仅拦 ArrowUp/Down（设 needRefresh + 调 `lathe.selectMold(selectedInput)`），其他事件 fallthrough 给 mainContainer 收 tick。**不要 `return true` 拦截 Event::Custom**。

访问器 `getComponent()`/`isRunning()`/`refresh()`（仿 [L783-789](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L783-L789)）。

---

## ASCII Art（全部纯基础 ASCII，符合 [L3](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L3) 乱码警告）

### 模具静态 icon（示例，实现时可微调）
```
Gear:     Rod:        Plate:        Spring:       Bolt:        Wire:
 .-"""-.   .----.     .=========.   /\ /\ /\ /\   /|  |\       .----.
|  o  |  (      )   /           \  /  V  V  V  \  \|__|/      (|||||)
 `-...-'   |      |  |   .---.     | \  /\ /\ /\ / /======\     |    |
           |      |   \           /   V  V  V  V    \======/   (|||||)
            `----'    `=========:                       |  |    `----'
                                                     |__|
```

### 插入动画（5 帧，500ms）—— 钢条 `[====]` 从左滑入卡盘 `|  |`
```
帧0: [====]            |  |     帧4:      [====] |==|  (卡盘闭合)
```

### 加工动画（4 帧循环，渲染时 `frameIndex/2 % 4` → 200ms/帧、800ms/循环，避免 100ms 闪烁）
```
   |  |   ~        --  |  |  ~        |  |  --  ~      ~  |  |  --
   |  | ~              |  |            |  | ~              |  |
   |__|                |__|            |__|                |__|
  /____\              /____\          /____\              /____\
 |      |            |      |        |      |            |      |
  \____/              \____/          \____/              \____/
```
`~` = 铁屑，`--` = 火花。

---

## 集成点（行号基于当前未编辑文件；建议自上而下编辑）

| # | 位置 | 改动 |
|---|---|---|
| 1 | [L331](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L331) | 插入 `Lathe` 类声明（~150 行） |
| 2 | [L393-395](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L393) 附近 | `tiles[8][5] = { 'L', "Lathe", "Machining facility", false, COLOR_PURPLE };` + reset `(5,6)`、`(5,7)` 为草地（避免随机树堵路）。坐标 (x=5,y=8)：与高炉 (5,5) 距离 3、汽车 (12,5) 距离 10，无相邻格重叠，无 E 键歧义 |
| 3 | [L499](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L499) | 修预存 passable bug：`tile.passable = ... && tile.display != 'F' && tile.display != 'L';`（读档后 'F'/'L' 才不会变可踩） |
| 4 | [L795](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L795) | 插入 `LatheUI` 类声明（~120 行） |
| 5 | [L801](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L801) 后 | 加 `Lathe lathe;` 成员 |
| 6 | [L879](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L879) 后 | newGame 加 `lathe = Lathe{};`；loadGame 的 `return gameMap.load(input);`（[L873](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L873)）前也加（读档清旧 job，比高炉更干净） |
| 7 | [L1193-1199](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1193) | `getItemPrice` prices map 加 `{"gear",25},{"rod",20},{"plate",18},{"spring",30},{"bolt",12},{"wire",35},{"shaft",45},{"ring",25}`（否则 trade 跳过这些产品，[L1186](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1186)） |
| 8 | [L1280](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1280) | 插入 `openLatheUI()`（完全镜像 [openFurnaceUI L1244-1279](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1244-L1279)：隐光标→Fullscreen→构造 LatheUI→ticker 线程 100ms PostEvent→screen.Loop→join→恢复光标→cls）。**不**调 `checkTutorialProgress`（见下） |
| 9 | [L1425](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1425) | 'E' 分支高炉检查后、汽车检查前插入：`if (currentArea==Area::Home && gameMap.isNear(5, 8, 'L', player.x, player.y)) { openLatheUI(); return; }`（`isNear` 签名 [L419](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L419)：`isNear(x,y,symbol,px,py)`） |
| 10 | [L1332](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1332) | 图例 `"F=Furnace C=Car a=Animal "` → `"F=Furnace L=Lathe C=Car a=Animal "` |

### 不做的事
- **不加教程步**：[L843](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L843) 的 `tutorialCount != tutorials.size()` 守卫会让所有旧存档读不出来。车床作为进阶设施让玩家自己发现即可。
- **不改存档格式**：`CHEMICAL_WORLD_SAVE 2`（[L811](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L811)）不含 furnace/lathe 运行态，读档后丢失当前 job 是预期行为（与高炉一致）。

---

## 复用的现有函数

- [`PlayerData::addItem/removeItem/hasItem/getItemCount`](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L73-L108) — steel 消耗与产品产出
- [`BlastFurnace::update`](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L265-L307) 的完成奖励+升级检查模式 — Lathe::update 直接镜像
- [`openFurnaceUI`](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1244-L1279) 的 ticker+Loop 模板 — openLatheUI 直接镜像
- [`FurnaceUI`](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L508-L789) 的 Maybe 门控 + CatchEvent tick + Renderer 分支 — LatheUI 直接镜像
- `gameMap.isNear`（[L419](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L419)）— E 键邻近判定

---

## 验证

1. **构建**：用 [Chemical-World.vcxproj](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.vcxproj)（Debug|x64，stdcpp20，FTXUI 在 `D:\360Downloads\ftxui-6.1.9-win64`）。VS 里 F5，确认无编译错误（重点：Lathe/LatheUI 在 ChemicalWorldGame 之前声明；LatheUI 引用的 `lathe` 成员存在）。
2. **新游戏**：出生 (8,5) → 走到 (5,7) → 按 E → 应打开车床 UI（不是高炉）。若开高炉说明坐标或 isNear 顺序错。
3. **缺料**：无 steel 按 LOAD → 显示 "Need steel for this mold."。
4. **给料**：先去高炉或 craft 菜单（C 键）造 steel，回车床。
5. **模具切换动画**：Idle 视图上下切 Radiobox → artBox 即时显示对应 icon。
6. **插入动画**：按 LOAD → artBox 切 5 帧滑入（~500ms）→ 自动进 Machining。
7. **加工动画+进度**：Machining 视图 artBox 循环 4 帧（铁屑/火花），进度条 0→100%，耗时 = mold.durationMs。
8. **完成**：切 Done 视图 → 显示结果 + COLLECT → 按回 Idle，artBox 回当前模具 icon。
9. **库存/经验**：看顶部 Inventory 出现 `gear x1`（或对应产品），Exp 增 mold.exp。
10. **每个模具**：重复 5-9 验证全部 6-8 种。
11. **取消**：Machining 中按 CANCEL → 回 Idle，steel 不退回（与高炉一致）。
12. **读档**：存档→读档→车床回 Idle、地图 'L' 仍在、玩家不能踩到 'L'（验证 #3 passable 修复）。
13. **图例**：Home 看地图下方应显示 "L=Lathe"。

---

## 风险与注意事项

1. **L499 passable bug**（#3）：不改则读档后玩家可走到 'L'/'F' 上。isNear 仍命中（查 display 不查 passable），功能不受影响但视觉怪。强烈建议改。
2. **CatchEvent 重入**：moldList 的 CatchEvent 只拦 Arrow 键（`return true`），Event::Custom 必须 `return false` fallthrough 给 mainContainer，否则 tick 收不到。
3. **帧速率**：ticker 100ms/tick。Inserting 5 帧=500ms 合适；Machining 4 帧若 100ms/帧偏快易闪，渲染用 `frameIndex/2 % 4`（200ms/帧、800ms/循环）。
4. **Maybe 与按钮焦点**：buttonCollect/buttonCancel 必须挂在 Container 里并用 `Maybe` 门控，不能只在渲染函数里 `->Render()` 手绘（这是 put-button bug 的教训，[L654-655](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L654) 注释）。
5. **ASCII 字符集**：仅用 `.-=/\|()~:_` 等基础 ASCII，无 Unicode 框线（避免 [L3](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L3) 乱码）。
6. **lambda 捕获**：LatheUI 所有 lambda 捕 `[this]`，`lathe`/`player` 是栈引用，openLatheUI 里 `screen.Loop` 同步阻塞、ticker join 后才析构，生命周期安全（同 [FurnaceUI L1254-1270](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1254-L1270)）。
7. **坐标 (5,8) 随机树遮挡**：已 reset (5,6)(5,7) 通道；其他路径可能被随机树挡需绕路，与高炉现状一致，可接受。
