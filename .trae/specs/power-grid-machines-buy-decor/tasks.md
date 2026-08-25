# Chemical-World 电力/建造/土高炉/购买/装饰 — 实现计划

## Task 1: 地图/存档基础设施（2×2 Tile、globalEU、蓝图、generators、v3 兼容）
- **Status**: `pending`
- **Priority**: high
- **Depends On**: None
- **Description**:
  - 在 Tile 存储不变（仍 display: char）的前提下，约定 2×2 机器用「大写主格 + 小写三格辅格」占位：F/f=BlastFurnace、L/l=Lathe、G/g=PowerGenerator。
  - `ChemicalWorldGame` 增加：`int globalEU = 0;`、`const int EU_MAX = 10000;`、`bool gen_blueprint_unlocked = false; bool wire_blueprint_unlocked = false;`、`map<pair<int,int>, PowerGenerator> generators;`（key 是左上 <x,y>）、`vector<MachineMeta> machineMeta;`（struct {int x,y; char type; int remainingBurnEU; int loadedCoal; bool active;}）。
  - `generate(Home)`：F 从 1×1 `tiles[5][5]='F'` → 2×2（F 5,5 / f 5,6 / f 6,5 / f 6,6，全部 COLOR_RED / COLOR_DARK_RED）。L 从 1×1 `tiles[8][5]='L'` → 2×2（L 8,5 / l 8,6 / l 9,5 / l 9,6，全部 COLOR_PURPLE / COLOR_DARK_PURPLE）。Car 保持 1×1。并显式 reset (5,5)–(6,6) / (8,5)–(9,6) 四格为机器，避免随机树。同时为玩家出生 (8,5) 原来的 L 所在格重置 → 玩家出生格 (10,5) 改为或调整保持原 (8,5) 附近的 walkable 通道。
  - `passable` 检查 L745–746：追加 `display != 'G' && display != 'g' && display != '+' && display != 'f' && display != 'l'`。
  - `renderMap` Home 地图：辅格 f/l/g 显示不同颜色（用 COLOR_DARK_*）以便肉眼观察 2×2 方块。图例追加 "G=Gen ~=River *=Flower v=Grass +=Wire"。
  - `saveGame(slot)`：版本 bump 到 `CHEMICAL_WORLD_SAVE 3`。在 map 保存后追加顺序写入：gen_blueprint_unlocked (0/1)、wire_blueprint_unlocked (0/1)、globalEU、machineMeta.size() → 每条 x,y,type,remainingBurnEU,loadedCoal,active；furnace.focusedSlot → 之后 4 条 slot 状态（active/loaded/progress/loadedOre/loadedCoal/recipeName）。
  - `loadGame(slot)`：读 header 后 `if (version == 2)` 用默认值初始化所有新字段（蓝图 false、globalEU 0、machineMeta 走一遍地图扫描找 F/L 左上格→塞入、generators 空）并跳过剩余读取；`if (version == 3)` 按上面 v3 顺序读。旧 tutorial 守卫不变。
  - `newGame()`：蓝图 reset 为 false、globalEU 0、machineMeta 清空并填入 F(5,5) 和 L(8,5) 两条默认 meta（type=F/L）。
- **Acceptance Criteria Addressed**: AC-2, AC-8
- **Test Requirements**:
  - `rule` TR-1.1: Build and run new game → Home map shows F/f 2×2 red block at (5,5) and L/l 2×2 purple block at (8,5), confirmed by visual inspection; player cannot walk onto them.
  - `rule` TR-1.2: Save game → file starts with "CHEMICAL_WORLD_SAVE 3"; create an old v2 save (or mock the first bytes) → LoadGame v2 does not crash; new fields default to expected values.
  - `rule` TR-1.3: passable('F')==false, passable('f')==false, passable('L')==false, passable('l')==false, passable('G')==false, passable('g')==false, passable('+')==false.
  - `rule` TR-1.4: Save after unlocking blueprints → reload → both bools roundtrip equal.
- **Notes**:
  - 由于 `isNear(5,5,'F', px, py)` 基于 display 字符，F/f 布局后若仅查 'F' 主格那么曼哈顿距离判定仍成立（目前实现查 display 匹配 + 曼哈顿距离 ≤ 2）。保持即可。
  - 文件写入用 `output << (int)bool_var` 读入用 `int tmp; input >> tmp; bool_var = !!tmp;` 避免 bool 读空格错。

---

## Task 2: PowerGenerator 类 + 电力网络辅助函数（powerDraw/powerAvailable + BFS 连通）
- **Status**: `pending`
- **Priority**: high
- **Depends On**: Task 1
- **Description**:
  - 在 BlastFurnace/Lathe 之前、PlayerData 之后插入 `class PowerGenerator`：
    - 私有：`int burnEU = 0; int loadedCoal = 0; int frameIndex = 0; bool active = false;`
    - 公共：`bool feedCoal(PlayerData&);`（consume 1 coal → burnEU += 6400，loadedCoal++，active=true，addLog-style 文本返回 string）；`pair<int,bool> updateTick(PlayerData&);`（返回 <注入EU, 是否还在燃烧>；burnEU>0 时 burnEU-=8 & frameIndex++，返回 <8,true>；否则返回 <0,false> active=false）；`int getFrameIndex(); int getBurnEU(); bool isActive();`
  - 在 ChemicalWorldGame 成员里放「全局电网 tick 函数」`void tickPowerGrid()`：
    - 每 tick（由 openFurnaceUI/openLatheUI/openBuildUI 的 100ms ticker 线程在发送 Event::Custom **前或后**一并调用，或通过 E 键交互面板调用）——实现方式：`ChemicalWorldGame` 增加 `void globalTick100ms()` 方法，里面先 `tickPowerGrid()`，再依次调用 `furnace.update()`、`lathe.update()`，**替代**原来分散在各 UI 的 update 调用。这样三个界面（F/L/B）的 CatchEvent 都只需要发一次 Custom 并调用 `globalTick100ms()`，电力/动画/进度统一推进。
    - `tickPowerGrid()` 逻辑：
      1. BFS 从 Home 地图所有发电机 'G' 主格位置出发 → 沿 '+' 电线四邻扩张，再扩张到 'F'/'L'/'G' 相邻（机器和电线上是通的）。
      2. 标记连通集合 `set visited_machines;`。
      3. 遍历每台 generator：调用 `auto [inj, burning] = generators[{x,y}].updateTick(player)`；若 burning 为 true `globalEU = min(EU_MAX, globalEU + inj)`；否则 `globalEU = min(EU_MAX, globalEU)`。
  - 辅助函数（文件级，放 Lathe 类之前）：`bool powerDraw(int euPerTick, PlayerData& player, const string& machineKey);`（machineKey 为 "LATHE" 等）：若 machine 在 BFS 连通集里且 `globalEU >= euPerTick`，就 `globalEU -= euPerTick; return true;`；否则 return false。连通集在 `tickPowerGrid()` 里每 tick 都计算并缓存为 `ChemicalWorldGame::set<pair<int,int>> poweredMachines`（set 里放机器左上 <x,y>）。`powerDraw` 查这个集合。
- **Acceptance Criteria Addressed**: AC-1
- **Test Requirements**:
  - `rule` TR-2.1: Feed 1 coal to a generator; check burnEU == 6400 immediately after.
  - `rule` TR-2.2: After 800 ticks (80 seconds) of updateTick calls with no further coal, burnEU shall reach 0 exactly (no negatives), active=false.
  - `rule` TR-2.3: With a wire path between G(15,5) and L(8,5), BFS in tickPowerGrid marks both machines in poweredMachines. Removing any single + wire tile in the only path makes L disappear from poweredMachines.
  - `rule` TR-2.4: powerDraw(2, player, "LATHE") when globalEU=42 → consumes 2, returns true, globalEU becomes 40.
- **Notes**:
  - BFS 可以只在 Home（currentArea==Home）下跑；Wasteland/Cave 里 globalEU 直接视为 0，保持简单。
  - `globalTick100ms()` 里调 furnace/lathe update 时要把旧的 `elapsedMs=100` 传进去兼容。

---

## Task 3: Home 固定装饰（河/湖/花/草丛）
- **Status**: `pending`
- **Priority**: medium（可和 Task1/Task2 并行，但依赖 generate 中 2×2 迁移已完成避免覆盖机器格）
- **Depends On**: Task 1
- **Description**:
  - 新增 `GameMap::placeRiver(int startX, int startY, int length, int direction)`（direction = HORIZ / DIAG_45）内部写 `'~' COLOR_BLUE !passable`，每次前进若不是空地 '.' 则终止并返回长度。若返回 <10 则重试。
  - 新增 `placeLake(int cx, int cy, int w=5, int h=3)` 以 (cx,cy) 为中心画 h 行 × w 列湖 '~' COLOR_DARK_CYAN。
  - 新增 scatter `placeFlowers(int count=40)` 和 `placeGrassTufts(int count=30)`：随机空位 `'*'`（COLOR_PURPLE/YELLOW/GREEN 循环，passable=true）与 `'v'`（COLOR_DARK_GREEN，passable=true）。
  - 调用顺序放在 `generate(Home)` 最后：先放机器（2×2 先占位，避免装饰覆盖）→ 再放河（尝试 3 条找 >=10 格的）→ 再放湖（离机器 >= 3 格）→ 再散花/草。
  - 每次 generate 都固定 seed 的效果不要求，但要保证数量下限（河 >=10、湖=3×5=15、花>=20、草>=15，两次种子都成立），与 F/L/C 无重叠。
  - renderMap 图例 Home 加 `~=River/Lake *=Flower v=Grass `。
- **Acceptance Criteria Addressed**: AC-4
- **Test Requirements**:
  - `rule` TR-3.1: Two consecutive NEW GAME runs produce Home maps each containing >=10 consecutive '~' river, >=15 '~' lake cells, >=20 '*', >=15 'v'.
  - `rule` TR-3.2: River/Lake/Flower/Grass cells never occupy the 2×2 rects of F/L/C or G candidate positions.
  - `rubric` TR-3.3: Dimension: decoration aesthetic diversity; scale 1-5; 1 = no river; 3 = river straight monotone; 5 = meandering river, irregular lake, mixed colors; threshold >= 3; evidence: screenshot of Home.
- **Notes**: 装饰全部固定生成、不可交互。不影响 passable 的机器检查。

---

## Task 4: B 键放大 7×7 建造界面（BuildUI + openBuildUI + 发电机 E 控制面板）
- **Status**: `pending`
- **Priority**: high
- **Depends On**: Task 1, Task 2, Task 3
- **Description**:
  - `class BuildUI`：仿 LatheUI/FurnaceUI 结构，持有 ChemicalWorldGame 引用以访问 `gameMap/player/蓝图/globalEU/generators`。
    - 成员：7×7 网格（49 个小 Renderer，每个点击后设 `selectedGX/specifiedGY` 并高亮 border）、Radiobox 选项列表「Generator / Wire / Flower / Grass tuft」、PLACE 按钮、CLOSE 按钮、顶部状态栏（金币/EU/蓝图解锁/已选项）、详情 Renderer（显示选中 tile display+name+passable+2×2 预览 if machine）。
    - 7×7 中心 = player.x, player.y → 左上 = (px-3, py-3)，右下 = (px+3, py+3)。超出地图边界的格子画 "OUT OF BOUNDS"。
    - PLACE：
      - Generator：检查 `gen_blueprint_unlocked`（未解锁提示 "Buy Generator Blueprint in Trade"）；检查 2×2 四格全为空地 '.'；检查 `player.coins >= 100`（建造成本 100c）；扣除 100c；写四格 G/g（G 左上 COLOR_RED，其他三格 g COLOR_DARK_RED 不 passable）；调用 `generators[{x,y}] = PowerGenerator{}`；加 machineMeta 一条。
      - Wire：检查 `wire_blueprint_unlocked`；检查单格空地 '.'；扣除 5c；写 '+' COLOR_BLUE 不 passable。
      - Flower/Grass tuft：不需蓝图，扣除 1c；写 '*' 或 'v' 可踩。
      - 任何一步失败 → statusMessage 明说原因。
    - 无 ticker（Build 界面不需要 100ms EU 更新；若想看电力数值可以顶部显读 globalEU 静态）。
  - `openBuildUI()`：只在 Home 打开；否则返回消息 "Can only build at home."；显关闭光标 → Fullscreen → Loop → 恢复 → cls。
  - `processKey case 'b': case 'B':` → 若 currentArea != Home → `message = "Build only available at home."; return;`；否则调用 `openBuildUI()`。
  - **发电机 E 控制面板**：在 processKey 的 E 分支里，F 检查、L 检查之后，追加 `if (currentArea==Home && gameMap.isNeighbourAnyOf(G, {15,5...}))`——实现成「遍历 generators map 每台 G，对每台左上 (gx,gy) 调 isNear(gx, gy, 'G', px, py)」；若命中则打开 `openGeneratorPanel(gx, gy)`（Console 小型面板或 FTXUI 小窗，含当前 remainingBurnEU / coal loaded / "ADD 1 COAL 按钮" / CLOSE）。添加煤时调用 `generators[{gx,gy}].feedCoal(player)`；不够 coal 提示 "Need coal."。
- **Acceptance Criteria Addressed**: AC-3
- **Test Requirements**:
  - `rule` TR-4.1: In B UI, pick Generator at (15,5) → with blueprint unlocked + >=100 coins + 2×2 empty → succeeds, writes 4 tiles; without blueprint → explicitly fails with the blueprint message.
  - `rule` TR-4.2: Wire placed at (10,5) → tile becomes '+' COLOR_BLUE and not passable.
  - `rule` TR-4.3: Press E beside a running generator → panel shows remainingBurnEU and offers ADD COAL; pressing it with 1 coal in inventory consumes 1 coal and adds exactly 6400 to remainingBurnEU.
  - `rubric` TR-4.4: Dimension: build UI visual clearness; scale 1-5; 1 = garbled; 3 = usable but uncolored; 5 = each of 49 cells bordered, colored per tile, blueprints and balance visible; threshold >= 4.
- **Notes**: 为简化，发电机成本（100c/台）、电线（5c/条）、装饰（1c）不写入价格表——只在 BuildUI 常量里。

---

## Task 5: 土高炉重制（4 槽并发 + 30s/批 + 12 帧炉膛动画）
- **Status**: `pending`
- **Priority**: high
- **Depends On**: Task 1
- **Description**:
  - **BlastFurnace 类全重写**：
    - 删除 temperature/heatTime。新增 `struct Slot { SlotState state = Idle; int elapsedMs = 0; int progress = 0; int loadedOre = 0; int loadedCoal = 0; int selectedRecipe = 0; int frame = 0; }; enum SlotState { Idle, Loading, Stoking, Heating, Pouring, Done }; Slot slots[4]; int focusedSlot = 0;`。
    - Recipe 表保持不变。
    - 构造函数填充 `stokingFrames(6)`（推煤入膛：煤块从右进炉膛）、`heatFrames(12)`（12 帧火焰 ASCII：'#' 红色火光、'~' 火星、'/'\\' 炉膛门光效、每帧偏移）、`pourFrames(6)`（铁水倒出：'v' 从炉底流下，每帧往下走一格）。
    - 公共方法：`void setFocusedSlot(int i)`、`int getFocusedSlot()`、`Slot& getSlot(int i)`、`vector<string> getFrameForSlot(int i)`（根据 slot.state + slot.frame 取对应帧）；保留 canLoad/loadMaterials/blowAir/cancel/getProgress/getStatus/getMaxProgress/isActive/isLoaded/getRecipeInfo/getRecipeName 等所有旧签名，统一转发到 `slots[focusedSlot]`，如超出范围或状态不匹配返回 false。blowAir 改成"对聚焦槽 addCoal 1 并推进到 Stoking 阶段（若 Idle），否则 nothing"。
    - `update(PlayerData& player, int elapsedMs = 100)`：对 slots[0..3] 每个槽：
      - Stoking：frame++，达到 6 → 转 Heating，elapsedMs=progress=frame=0。
      - Heating：frame++；elapsedMs+=elapsedMs；progress=min(100, elapsedMs*100/30000)；达到 30s → 转 Pouring。
      - Pouring：frame++，达到 6 → 产出 `player.addItem(recipe.result, count, "product", 20)` + 奖励 15 exp + 升级检查 + addLog + 转 Done。
    - 每槽独立。旧 `progress` 字段 → `slots[focusedSlot].progress`；`getStatus()` → 返回 "S0:" + state 等简写一行。
  - **FurnaceUI 重构**：
    - 顶部增加 `slotTabs` Radiobox（"Slot 0"/"Slot 1"/"Slot 2"/"Slot 3"），切换 focusedSlot。
    - 炉体中央 Art Renderer 显示 `furnace.getFrameForSlot(selectedSlotIndex)`。
    - 保留原 recipeList Radiobox。
    - 按钮：「LOAD SELECTED SLOT」（调 canLoad→loadMaterials，返回状态消息）、「ADD ORE 1」「ADD ORE -1」「ADD COAL 1」「CANCEL SLOT」、旧 BLOW AIR 去掉或变成「STOKE」按钮。
    - 底部 LogViewer 合并 4 槽日志（每条前缀 "[S0]" 之类）。
    - Maybe 门控不变（按 putting 投料视图切换）。
    - 动画 + 进度推进由 `ChemicalWorldGame::globalTick100ms()` 统一调用 `furnace.update(player)`，CatchEvent 不再直接调。
  - **无电力依赖**：update() 内不出现 powerDraw 或 globalEU。
- **Acceptance Criteria Addressed**: AC-6
- **Test Requirements**:
  - `rule` TR-5.1: 4 slots simultaneously loaded with Steel recipe all 4 start Heating within 1 second of each click; all 4 complete pouring between 30.0s and 31.0s, adding 4 steel to inventory.
  - `rule` TR-5.2: The focused slot's furnace art cycles through 6 stoking frames, then 12 heating frames (looping), then 6 pouring frames; total of 24 distinct visual frames per batch.
  - `rule` TR-5.3: Lathe UI / Build UI does not show any "Power required for furnace" or EU consumption lines for furnace actions; furnace update never touches powerDraw/globalEU.
  - `rubric` TR-5.4: Dimension: animation appeal; scale 1-5; 1 = no animation; 3 = simple three steps; 5 = stoking shows coal moving, heating shows flame sprites cycling, pouring shows iron flow; threshold >= 4.
- **Notes**:
  - 30s 单批意味着 4 批 steel 总奖励 exp = 4 × recipe.exp，保持 exp 不变。
  - `blowAir` 旧外部调用点（如果有）需要改为 "聚焦槽 ADD COAL 1"。若没有其他调用，可直接删除。

---

## Task 6: Lathe 耗电改造 + UI 顶栏 EU/Burning 显示
- **Status**: `pending`
- **Priority**: high
- **Depends On**: Task 1, Task 2
- **Description**:
  - `Lathe::update(PlayerData& player, int elapsedMs = 100)`：在 `Machining` 分支开头加：
    ```cpp
    if (animState == Machining) {
        // 需要电力连通（lathe 左上 (8,5) 在 poweredMachines）+ 扣 2 EU
        if (!powerDraw(2, player, {8,5}, "LATHE")) {  // 新签名：位置+名字
            if (isRunning) {
                addLog("X No power. Lathe paused.");
                isRunning = false;
            }
            return false;
        }
        isRunning = true;  // 有电后恢复
        ...
    }
    ```
    `powerDraw` 扩展签名为 `bool powerDraw(int eu, PlayerData&, pair<int,int> machineXY, const string& name)`：先查 `poweredMachines.contains(machineXY)`，通过后查 globalEU >= eu，扣，返回 true。
  - Idle/Inserting/Done 不耗电、不调用 powerDraw。
  - **LatheUI 顶部状态区**：Idle 主视图顶部在 Status/Mold 行之上追加两行：
    - `Power: globalEU / 10000 EU`（实时显示，每次重绘读 globalEU）
    - `Burning gens: <n> / <total>`（遍历 generators map 统计 burning == true 的数量）。
  - **无电 UI 反馈**：若 Machining 且 isRunning==false（已停），statusMessage 改红显示 "⚠ PAUSED: No power. Feed coal at generators."，进度条颜色改为 Grey。
- **Acceptance Criteria Addressed**: AC-5
- **Test Requirements**:
  - `rule` TR-6.1: Start lathe machining with 40 EU bank; run 20 ticks (2s) without replenishment → progress value at tick 20 is exactly the same as at tick 21 (frozen), and lathe log contains "No power. Lathe paused.".
  - `rule` TR-6.2: After feeding coal to a connected generator, within 2 seconds lathe.isRunning becomes true again and progress increases by >=2 in next 10 ticks.
  - `rule` TR-6.3: Lathe Idle state does not consume EU (globalEU unchanged for 100 ticks while idle).
- **Notes**: Lathe 左上硬编码 (8,5) 因为 Task1 已经把它迁移到固定 2×2 占位。

---

## Task 7: TradeUI 顶部 Tab（SELL/BUY）+ BUY Tab 条目和蓝图解锁
- **Status**: `pending`
- **Priority**: high
- **Depends On**: Task 1, Task 6
- **Description**:
  - **Tab 容器**：TradeUI setupUI 里新建 `int tabSelected = 0;`；创建 Radiobox `tabs`（{ "SELL", "BUY" }, &tabSelected）放在最顶部。主 layout 里先 hbox 放 tabs，再用 `Maybe` 根据 `tabSelected==0/1` 切换显示 sell 内容或 buy 内容。
  - **SELL Tab**：保持现有实现（menu 背包 + SELL ONE/ALL/EVERYTHING/CLOSE + 详情），只是 Maybe 条件 `[this]{return tabSelected==0;}`。
  - **BUY Tab**：
    - 左侧 `buyMenu`：列出可买条目，固定顺序：
      ```
      0: raw_ores: hematite(15c), magnetite(15c), bauxite(12c), cassiterite(15c), malachite(18c), chalcopyrite(18c), gold_ore(60c), silver_ore(36c)
      1: fuels/materials: coal(24c), sand(6c), glass(45c), steel(90c), iron_ingot(36c), alloy(135c)
      2: steel_parts: steel_gear(75c), steel_rod(60c), steel_plate(54c), steel_spring(90c), steel_bolt(36c), steel_wire(105c)
      3: blueprints: Generator Blueprint(150c), Wire Blueprint(50c)   <- 这两条 price 独立于 itemPrice() 定价
      ```
      每个物品买入价 = itemPrice(name) * 3；蓝图独立写 150c / 50c。
    - 右侧 `buyDetail`：类似 sell 详情，名字、价格、按钮 BUY 1。
    - BUY 1 行为：
      - 普通物品：coins >= price → 扣 coins、`player.addItem(name, 1, "material", itemPrice(name))`；否则 statusMessage "Not enough coins."
      - 发电机蓝图：coins >= 150 且 !gen_blueprint_unlocked → 扣 150 → set true + menu 条目改成 "[OWNED] Generator Blueprint"且按钮 disabled。
      - 电线蓝图：coins >= 50 且 !wire_blueprint_unlocked → 扣 50 → set true + 显示 "[OWNED] Wire Blueprint"。
    - CLOSE 按钮 Sell/Buy 共享。
  - **重建 sell/buy 菜单**：每次点击 BUY/SELL 后 `rebuildSell()`/`rebuildBuy()` 把 entries / prices / names 刷新。
- **Acceptance Criteria Addressed**: AC-7
- **Test Requirements**:
  - `rule` TR-7.1: Start with coins=10000 → BUY Tab buys 1 hematite at itemPrice*3, coins correctly decreases by 15; inventory.getItemCount("hematite") increases by 1.
  - `rule` TR-7.2: Buy Generator Blueprint (150c, before locked) → gen_blueprint_unlocked becomes true AND after restarting trade UI row shows "[OWNED]" AND button clicks do not decrease coins anymore. Same check for Wire Blueprint (50c).
  - `rule` TR-7.3: Switching Tabs from SELL→BUY→SELL does not lose selections or crash for at least 5 quick switches.
  - `rubric` TR-7.4: Dimension: BUY visual layout; scale 1-5; 1 = overlapping; 3 = lists only; 5 = groups by category, blueprint rows visually distinct (e.g. cyan color); threshold >= 4.
- **Notes**: itemPrice(蓝图名) == 0 但 BUY 定价单独处理，不走 itemPrice*3 公式，单独写死常量 150/50。

---

## Task 8: 全局整合 + processKey B/L/E 调度 + 构建验证
- **Status**: `pending`
- **Priority**: high
- **Depends On**: Tasks 1-7 全部
- **Description**:
  - processKey：加 `case 'b': case 'B'` 分支；在 E 分支 F 检查 / L 检查后追加「对每台发电机 G 做 isNear(gx, gy, 'G', px, py) → openGeneratorPanel(gx, gy)」。
  - 把三个 UI（FurnaceUI/LatheUI/BuildUI/GeneratorPanel FTXUI 全屏）的 CatchEvent(Event::Custom) 统一改成调用 `globalTick100ms()`，且在 ticker 线程保持 100ms 不变。
  - 把 `ChemicalWorldGame::globalTick100ms()` 实现：顺序调用 `tickPowerGrid()` → `furnace.update(player)` → `lathe.update(player)`。注意：furnace.update 里内部每槽循环，lathe.update 里调用 powerDraw，都依赖 tickPowerGrid 结果。
  - 主界面（非全屏 UI）不需要 tick，也不耗电，保持原样。
  - 图例 Home 行和帮助文本同步。
  - MSBuild Debug|x64 构建，Exit=0。
- **Acceptance Criteria Addressed**: AC-2 (E 调度)、AC-9（构建）
- **Test Requirements**:
  - `rule` TR-8.1: Press B at Wasteland → shows "Can only build at home." message only; no crash.
  - `rule` TR-8.2: Press E next to F/L/G each opens the correct panel.
  - `rule` TR-8.3: MSBuild Debug|x64 returns EXIT_CODE=0 with zero `error CXXXX`.
- **Notes**:
  - generator panel 如果用 FTXUI 全屏，就也要有 openGeneratorPanel 函数（镜像 openFurnaceUI，简单但 ticker 可以去掉，因为只投煤不需要动画）；若用 console small panel 也可以，确保不闪烁即可。
  - 如果多个 generator 同时在玩家距离 2 格内，E 只打开第一个（遍历 map 的顺序），避免冲突。
