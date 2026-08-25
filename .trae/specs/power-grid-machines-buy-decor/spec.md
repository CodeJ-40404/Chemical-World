# Chemical-World 电力 + 建造 + 土高炉重制 + 贸易购买 + 家园装饰 PRD

## Overview
- **Summary**: 在现有 Chemical-World 单文件 C++ 游戏里追加 7 个互锁模块：(1) 数字电力网络；(2) B 键 FTXUI 放大建造界面（装发电机/电线）；(3) Home 固定生成河流/湖泊/花草装饰；(4) 车床耗电改造；(5) 土高炉 4 批并发+30 秒+12 帧动画+无电力；(6) TradeUI 增加「购买」Tab（原材料 + 钢零件 + 机器蓝图，售价 ×3）；(7) Tile/存档 扩展以承载 2×2 机器占位、蓝图库存和电力状态。
- **Purpose**: 让玩家从「采矿-高炉-车床-贸易」线性链进入「造发电机-布电线-喂煤-跑机」的 GT:NH 风格循环，并通过家园装饰和购买界面补充长期留存内容。
- **Target Users**: 单人终端游戏玩家，已熟悉 WASD + E 交互、F/T 菜单。

## Goals
- 玩家默认赠送火力发电机（需先在 Trade 购买「发电机蓝图」+「电线蓝图」才能用 B 键建造）。
- 车床加工时必须有电力；无电立刻停止（进度保留）。
- 土高炉从单批 20s + 电力无关 → 升级为 4 批并发 30s + 12 帧炉膛动画，无电力依赖（土高炉 = 不烧电）。
- B 键打开 7×7 FTXUI 放大建造界面，点击空格放置机器/电线/装饰；机器 2×2 方块占位，电线/装饰 1×1。
- Home 区域每次 generate() 时自动生成一条 10–15 格河流、一个 3×5 湖泊、散布花/草丛。
- TradeUI 顶部增加 Tab「SELL / BUY」，BUY 卖原材料 + 钢零件 + 发电机/电线蓝图，买入价 = 对应物品出售价 ×3，蓝图只能买 1 次/永久解锁。

## Non-Goals
- 变压器、EU 等级、电力损耗、多电压网络（保持"简单数字网络"）。
- 水坝、风力、太阳能等其他发电机（用户明确本次只做火力）。
- 购买 Tab 的购买数选择器（SELL ONE / SELL ALL 分离）不做——购买一次数量 = 1，重复按即可。
- 玩家携带的多台发电机/多蓝图库存堆叠。蓝图为「已解锁/未解锁」布尔标志，不是可堆物品。
- 河流有鱼、湖泊可装水泵等互动——装饰纯视觉，不可交互。
- 电网跨 Area 供电（仅 Home 通电，其他区域没有电网）。
- 放置拆除/退还机器（放了就永久存在，不做拆除按钮）。

## Background & Context
- 技术栈：FTXUI 6.1.9（`D:\360Downloads\ftxui-6.1.9-win64`），C++20，Windows Console API（`_getch`、`SetConsoleTextAttribute`、conio.h），MSBuild（`D:\c++\for-vs\MSBuild\Current\Bin\MSBuild.exe` Debug|x64）。
- 现有地图尺寸：Home 60×50、Wasteland 150×80、Cave 45×28。`Tile` 结构 `{display, name, desc, passable, color}`，display 为 1 个 char，`tiles[y][x]` 索引。
- 现有键位（processKey L2050–L2112）：WASD 移动、E 交互、C Craft 菜单、T 交易、F 提示、H 帮助、Q 退出。**B / L 未用**。B 用作 Build 放大界面键位。车床用电的"L 需要电力"指 Lathe 类本身（不是 L 键位）。
- 现有 F=BlastFurnace 放 `tiles[5][5]`、C=Car `tiles[5][12]`、L=Lathe `tiles[8][5]`，均为 1×1。本 PRD 改它们为 2×2 方块（左上原点保持 x=5,y=5 等），并调整 isNear/E 交互。
- 现有 FTXUI 全屏打开模板：`openFurnaceUI()`/`openLatheUI()`：隐光标→`ScreenInteractive::Fullscreen()`→UI 类→ticker 线程 100ms `PostEvent(Event::Custom)`→`screen.Loop()`→join→恢复光标→`cls()`。B 键建造界面严格镜像此模板。
- 存档版本 `CHEMICAL_WORLD_SAVE 2`：player 基础信息 + area + inventory（`name qty category value`，名字里**不能有空格**，因为 `input >> item.name` 读）+ tutorial bool 数组 + map（逐 tile 存 `display mineral richness hits color`）。本 PRD 必须 bump 版本到 3，并追加：蓝图解锁位、2×2 机器位 2、电力总 EU、每台发电机的剩余煤/燃烧 EU。
- 土高炉现状：BlastFurnace 单批、有温度/heatTime、100ms tick 驱动 update()。本 PRD 重写为 4 个独立槽位 + 30s/批 + 12 帧 ASCII 炉膛动画。
- 购买/蓝图：蓝图定义为 `gen_blueprint_unlocked`、`wire_blueprint_unlocked` 两个 bool（放在 ChemicalWorldGame 主状态里），存读档序列化；TradeUI BUY 按钮点击时若未解锁则花金币解锁，已解锁则按钮灰化+显示"已拥有"。

## Functional Requirements
**FR-1 (电力)**:
  - 全局 `globalEU` 数值（int；初始 0，上限 10000 EU 缓冲）。
  - `PowerGenerator` 类：接收 `feedCoal(PlayerData&)`（consume 1 coal → 内部 `burnEU += 6400`），每 100ms tick 调 `update()`：若 `burnEU>0` 则向 `globalEU` 注入 8 EU，`burnEU -= 8`；并推进 `frameIndex`。
  - `powerAvailable(consumptionEUperTick = 0)` 辅助函数：若 `globalEU >= consumptionEUperTick` 返回 true，并在内部做「尝试扣减 EU」动作（返回 true 时真扣，false 不扣）。
  - 电线与通电范围：仅 Home。当玩家在 B 键放置了 1×1 display='+' 电线时，BFS 搜索「所有 2×2 机器 + 所有发电机 + 相邻电线格」组成的连通图。只要连通图里至少包含 1 台正在燃烧 EU 的发电机，图里所有 2×2 用电机器就能消费 `globalEU`。这实现"简单数字网络 + 电线相邻即连通"。
  - `Lathe` 加工期间每 tick 调用 `powerAvailable(2)`（20 EU/s = 2 EU/tick@100ms），拿不到电时 `isRunning=false` 冻结进度，`frameIndex` 停住，log "X No power: lathe paused"。

**FR-2 (B 键放大建造)**:
  - `case 'b': case 'B':` 仅在 `currentArea == Area::Home` 时打开 `openBuildUI()`；否则提示"只能在家园建造"。
  - `openBuildUI()` 镜像 openFurnaceUI ticker 模式。界面为 FTXUI 全屏：顶部状态（余额 / EU / 蓝图解锁）、中部 7×7 网格（以玩家脚下 (px,py) 为中心，网格单元用 border + char + color 渲染）、右/底部 Radiobox 选项列表「发电机 / 电线 / 花 / 草丛」、"PLACE"按钮、"CLOSE"按钮。
  - 点击 7×7 某格后 PLACE：若选的是「发电机」，则以该格为左上，要求 2×2 四格都是空地 `'.'` 且没与 F/L/C/已有机器 / 地图边界碰撞，同时蓝图已解锁 + 玩家至少 1 coal（启动燃料）；满足则写四格 tiles：`tiles[y][x] = 'G'`（发电机本体，不 passable）、另外三格 `'g'`（辅助块，不 passable），并在 `generators[{x,y}]` 注册表（map 键是左上 x,y）。不满足 → statusMessage 提示具体原因。
  - 放「电线」：只需要 1×1 空地 + 蓝图解锁 → display='+' 不 passable + COLOR_BLUE（电线 1×1）。
  - 放「花/草丛」：1×1 空地、不需要蓝图 → display='*' / 'v'、passable=true（装饰可踩）。
  - 「CLOSE」按钮关闭界面 → `cls()`。
  - 主游戏地图 passable 检查：`display != '=' && display != 't' && display != 'F' && display != 'L' && display != 'G' && display != 'g' && display != '+' && display != '#';`（'+'电线/机器不能踩）。
  - 机器 2×2 迁移：
    - BlastFurnace 从 `tiles[5][5] = 'F'` 1×1 → 改成 2×2：`tiles[5][5]='F'`，`tiles[5][6]='f'`，`tiles[6][5]='f'`，`tiles[6][6]='f'`（都 passable=false）。isNear 保持 `isNear(5,5,'F',...)`（isNear 只查 display 不查曼哈顿即可——现有实现匹配）。
    - Lathe 从 1×1 `tiles[8][5]='L'` → 2×2：`tiles[8][5]='L'`，`tiles[8][6]='l'`，`tiles[9][5]='l'`，`tiles[9][6]='l'`，都 passable=false。
    - Car `tiles[5][12] = C` 保持原样（1×1 passable=true）。
    - generate() 为迁移后的机器显式 reset 4 格、并为玩家出生 (8,5) 绕车/高炉清通道。
  - save/load 保存新增 2×2 元信息（`machineMeta`：左上 x,y、类型 'F'/'L'/'G'），保证读档时地图 2×2 元块还能被 BlastFurnace/Lathe/Generator 逻辑定位。

**FR-3 (家园装饰)**:
  - `generate(Home)` 在 fill + 随机树之后追加：一条长度 12 的水平或 45° 折线河（display='~'，COLOR_BLUE，不 passable）、一个 3 行 × 5 列湖（display='~'，COLOR_DARK_CYAN，不 passable）、40 朵随机花（'*'，COLOR_PURPLE/YELLOW/GREEN 循环，passable）、30 丛草（'v'，COLOR_DARK_GREEN，passable）。
  - 固定生成的河/湖不与 F/L/C 4 格占位、G 起始推荐位置（如 (15,5)）相撞。放置时随机选一个 x/y 起点，若 4×4 范围内有机器/出生点/汽车图块就重试。
  - 图例：`renderMap` Home 区加 `~=River/Flower` 说明。

**FR-4 (土高炉重制)**:
  - `BlastFurnace` 改成 4 槽位：`struct Slot { bool active; bool loaded; int progress; int loadedOre, loadedCoal; Recipe recipe; } slots[4];`。
  - UI：原 `recipeList` Radiobox 保留（选目标配方），下面增加 4 个槽位框 + 槽位 Radiobox（选当前操作哪个槽），加「LOAD SELECTED SLOT」「ADD COAL 1」「ADD ORE N (ORE±)」「CANCEL SLOT」按钮。
  - 时间：单批 30s（`maxProgress=100，每 tick (100ms) + 0.333 等价于 elapsedMs*100/30000`）。
  - 动画：新增 `stokingFrames(6) + heatFrames(12) + pourFrames(6)`。12 帧炉膛循环 `heatFrames`（参考 GTNH：燃烧火焰 ASCII、炉门光效、火星四溅 `~` 位置周期性变化）。主界面 render 时显示 `slots[focusedSlot]` 当前状态对应的帧序列。
  - 每槽状态机：`Idle(未装料) → Loading(装料中) → Stoking(推煤入膛 6 帧/600ms) → Heating(升温 12 帧循环 + 进度) → Pouring(铁水出炉 6 帧/600ms) → Done(产物入库)`。4 槽相互独立，可同时运行。
  - 无电力依赖：`update()` 与 globalEU 无关。
  - 旧 BlastFurnace 所有外部调用点（`loadMaterials(player)`/`update(player)`/`canLoad`/`blowAir`/`cancel`/`getProgress`/`getStatus`/`isActive` 等）重构为「默认操作 slots[focusedSlot]」，槽位不足/状态不合法时返回 false+log。`blowAir` 改为"对当前槽 addCoal 1"兼容旧动作（若在 Stoking/Heating 中）。

**FR-5 (车床耗电)**:
  - `Lathe::update(PlayerData&, elapsedMs)` 里在 `Machining` 分支最前加 `if (!powerDraw(2, player)) { addLog("X No power."); return false; }`。`powerDraw(2EU, player)` 调 `powerAvailable(2EU)`——若 globalEU 够就扣 2EU 并返回 true，不够返回 false，**且** `isRunning=false` 冻结进度、`animState` 保持 `Machining`（下次再来电自动解冻，恢复加工）。
  - Idle/Inserting/Done 不耗电。
  - UI 顶部状态栏加"Power: globalEU / 10000 EU"和"Burning: ?kW"文字。

**FR-6 (TradeUI BUY Tab)**:
  - `TradeUI` 顶部 Container::Tab 加 `tab_sell` / `tab_buy` 两个 Tab（用 FTXUI 的 `Container::Tab` 组件或手工 Radiobox 切换）。
  - SELL Tab 维持现有实现（SELL ONE / SELL ALL / SELL EVERYTHING）。
  - BUY Tab：右侧按钮改为「BUY 1」「CLOSE」；条目 = `{raw ores/coal/sand/glass/steel/ingot/alloy, steel_gear/rod/plate/spring/bolt/wire, 发电机蓝图(仅限1次), 电线蓝图(仅限1次)}`，单价 = `itemPrice(name) * 3`（或蓝图底价 150c / 50c，蓝图不属 itemPrice）。
  - 选中后点 BUY 1：金币够则 `player.coins -= price`。若是物品则 `player.addItem(name, 1, "material", itemPrice(name))`；若是蓝图则 `gen_blueprint_unlocked = true` 或 `wire_blueprint_unlocked = true`。购买完成后 `statusMessage` 显示 "+1 " + name。蓝图已解锁的条目显示 "[OWNED]"，按钮灰化。

**FR-7 (存档兼容)**:
  - 版本号 bump 为 `CHEMICAL_WORLD_SAVE 3`。
  - 在原有 inventory/tutorial/map 之后写入：蓝图解锁位（2 个 bool）、globalEU、machineMeta 数量 N → 每个 `x y type remainingBurnEU loadedCoal active`、furnaceFocusedSlot、furnace slots[4] 状态。
  - `loadGame` 里：若 version==2 则用默认值填充新字段（蓝图未解锁、globalEU=0、machineMeta 按地图 tiles 扫描 F/L 推断；发电机没有则为空），保持旧档可读。若 version==3 按新格式读。
  - `tutorials` 数量不变（不新增教程步，避免破坏 L865 守卫）。

## Non-Functional Requirements
- **NFR-1 性能**: 每 tick（100ms）内全局电网 BFS + 4 槽位 update + ASCII 帧推进必须在 <16ms 完成（60×50 地图下 BFS 节点数通常 < 100，完全可行）。
- **NFR-2 稳定**: 不引入任何内存泄漏；所有 FTXUI Component 引用对象在 UI 生命周期内有效；保存任何版本都不崩溃。
- **NFR-3 乱码**: 所有新增文本、ASCII 艺术都用基础 ASCII（A-Za-z0-9 `!@#$%^&*()_+-=[]{};':",./?|\\<>`），禁止 Unicode 框线字符。
- **NFR-4 代码一致性**: 每个新类/结构的格式与现有 `BlastFurnace`/`Lathe`/`FurnaceUI`/`LatheUI`/`TradeUI` 对齐（缩进、注释、私有-公共顺序、日志、方法签名）。
- **NFR-5 构建**: MSBuild Debug|x64 零 error。

## Constraints
- **Technical**: 只改 `Chemical-World.cpp` 单文件；不得引入第三方库。FTXUI 版本锁定 6.1.9；不得使用 Canvas / animation.hpp 等未验证组件。
- **Business**: 不得破坏 F/T 热键入口；F 界面打开方式保持 E 邻近打开；土高炉完成要产出 steel 且奖励 exp 不变；T 键仍默认打开 TradeUI。
- **Dependencies**: B 键建造依赖蓝图购买（FR-6）；车床耗电依赖电力网络（FR-1）；土高炉重制依赖 4 槽位 + 动画迁移（FR-4）。

## Assumptions
- 1 EU = 100ms 能量单位。`8 EU/tick = 80 EU/s`。
- 发电机占 2×2，显示主格 'G' 红，辅助 'g' 暗红；不占高炉/车床/汽车图块。
- 电线占 1×1，显示 '+' 蓝。相邻连通：上下左右四邻。
- 蓝图只买 1 次 → 永久解锁。
- 玩家只能在家园建造；Wasteland/Cave 按 B 提示返回家园。

## Acceptance Criteria

### AC-1: 火力发电机 + 电力网络（简单数字）
- **Type**: `rule`
- **Given**: 新游戏或读档后，玩家在 Home (15,5) 用 B 建造了一台发电机，并在 Trade 用 BUY 解锁了电线蓝图，且在发电机和车床之间放置了一条连续的相邻 '+' 电线。
- **When**: 发电机喂入 1 个 coal（例如在 Build 里 FEED COAL 按钮或发电机 E 交互面板喂煤）。
- **Then**: `globalEU` 缓冲从 0 涨到 >= 6400 EU；车床开机加工后每 tick 扣 2 EU；车床能完成一次 job；若喂完 coal 后等 80s（6400EU ÷ 80EU/s = 80s）内不再加煤，`burnEU` 归 0，车床遇到电力不足立刻停。
- **Pass Condition**: 连续 2 次完整加工车床 cycle 都不中途"卡死进度"; burnEU 耗尽后 lathe.addLog 出现 "X No power."；EU 足够后恢复加工。
- **Evidence**: MSBuild 编译通过；手动日志截图或 UI 实时面板 Power/Burning 数值核对。

### AC-2: 2×2 机器占位与 E 邻近交互兼容
- **Type**: `rule`
- **Given**: Home generate 完成。
- **When**: 玩家从 (5,3) 按 E。
- **Then**: 打开 BlastFurnace 界面；玩家走到 (8,4) 按 E 打开 Lathe 界面；玩家走 (5,7) 或 (14,4) 附近时踩不到 'F'/'f'/'L'/'l'/'G'/'g'/'+' 图块。
- **Pass Condition**: 所有 passable 检查阻挡这些 display；isNear 返回正确。
- **Evidence**: `renderMap` 上机器方块 2×2 连续颜色块可观察 + 步行碰撞测试。

### AC-3: B 键放大 7×7 建造界面
- **Type**: `rubric`
- **Dimension**: 建造可用性与视觉
- **Scale**: 1-5
- **Anchors**: 1 = 打不开或崩溃；3 = 能放置但无颜色/无细节；5 = 7×7 网格每格有 border + 显示对应 tile display + color；点击后菜单清晰，PLACE 成功/失败消息明确。
- **Pass Threshold**: >= 4
- **Evidence**: 放 2×2 G 发电机、单格 + 电线、单格花 * 各 1 次，截图+状态消息+碰撞。

### AC-4: Home 自动装饰（河/湖/花/草丛）
- **Type**: `rule`
- **Given**: 新游戏两次（两次独立种子）。
- **When**: 启动进入 Home。
- **Then**: 两次地图里都出现 1 条连续 '~' 河（>= 10 格）、1 个 3×5 '~' 湖块、>= 20 朵 '*' 花和 >= 15 丛 'v' 草丛，不与 F/L/C/G 机器占位重叠。
- **Pass Condition**: 两次 generate 都满足。
- **Evidence**: renderMap 肉眼观察或 Select-String 统计 display count。

### AC-5: Lathe 耗电自动暂停/恢复
- **Type**: `rule`
- **Given**: 车床正在 Machining，globalEU 有 40 EU。
- **When**: 等 2 秒无新煤（40EU / 2EU = 20 tick → 2s 燃尽）。
- **Then**: 第 21 tick 开始 `globalEU=0`，Lathe `isRunning=false`，progress 保持，animState 仍 Machining，log "X No power."；立即喂 1 coal → 2 秒内 globalEU>0 → Lathe `isRunning=true`，进度继续向上。
- **Pass Condition**: 手动两次完整循环（停→补煤→继续）通过。
- **Evidence**: LatheUI 进度条停在某值、恢复后平滑增长。

### AC-6: 土高炉 4 槽并发 + 30s/批 + 12 帧动画
- **Type**: `rule`
- **Given**: 玩家有 4 份 hematite 和 4 份 coal，F 界面切到 4 个槽位各装 1 steel 配方。
- **When**: 依次启动 4 个槽（允许间隔 1s）。
- **Then**: 4 个槽进度条独立前进；炉膛动画每 100ms 推进 1 帧，共 12 帧循环；30s 后 4 个槽分别产出 steel ×4；土高炉面板无耗电字段。
- **Pass Condition**: 4 个槽各自在 30s±1s 内完成；动画不卡顿；无电力字段。
- **Evidence**: FurnaceUI 截图 + 日志时间戳。

### AC-7: TradeUI BUY Tab 原材料+钢零件+蓝图
- **Type**: `rule`
- **Given**: 新游戏，player.coins = 10000。
- **When**: 按 T → BUY Tab → 依次购买 hematite×1、coal×1、steel_gear×1、发电机蓝图、电线蓝图。
- **Then**: 对应物品进入 inventory；`gen_blueprint_unlocked=true` 且蓝图条目显示 "[OWNED]" 无法再次点击；金币 = 10000 - sum(prices) 且价格 = 对应 itemPrice ×3；B 键建造时发电机/电线蓝图检查通过。
- **Pass Condition**: 5 项购买行为全部成立；金币余额正确。
- **Evidence**: TradeUI BUY Tab 金币/条目确认；B 键 Build 界面蓝图解锁状态和实际一致。

### AC-8: 存档 CHEMICAL_WORLD_SAVE 3 前后兼容
- **Type**: `rule`
- **Given**: 旧存档 version=2（例如 Chemical-World/save1.sav 若存在）；或创建一份 v2 存档。
- **When**: 用新代码 LoadGame v2 → 加一个发电机 → 放电线 → SaveGame（应变成 v3）→ 再次 LoadGame v3。
- **Then**: LoadGame v2 不崩溃；蓝图、globalEU、machineMeta 使用默认值；LoadGame v3 后发电机仍在地图上、`remainingBurnEU` 恢复、车床蓝图解锁保留。
- **Pass Condition**: 无 crash；状态完全恢复。
- **Evidence**: 保存前后文件内容（前 200 字节对比）+ 游戏内打开 B/F/L 界面无错误。

### AC-9: 构建零错误
- **Type**: `rule`
- **Given**: 源码完成所有改动。
- **When**: 运行 `MSBuild Chemical-World.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal /m`。
- **Then**: Exit code = 0；无任何 `error CXXXX`。
- **Pass Condition**: EXIT=0。
- **Evidence**: Shell 命令输出尾部和 EXIT。

## Open Questions
- [ ] **Open-1**: 发电机的煤怎么投放？（A. Build 界面放置发电机就自动消耗 1 coal 作为首充；B. E 与发电机相邻打开小型控制面板，面板里"ADD COAL"按钮；C. 两者都做）→ 本 PRD 暂按 **B（单独控制面板）**，若用户未作回复即默认 B。
- [ ] **Open-2**: 购买界面的蓝图价位？（发电机蓝图=150c、电线蓝图=50c 是否合适？）→ 本 PRD 暂按 150/50，若用户未作回复即默认。
- [ ] **Open-3**: 是否允许删除已放置的机器/电线？（Non-Goals 目前禁止；如用户希望有"U 键拆除返还半额资源"请回复）→ 本 PRD 暂按不可拆。
