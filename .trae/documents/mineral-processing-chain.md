# 矿物处理链（GT:NH 风味）实施计划

## Context

当前游戏矿物流程过于线性：挖矿（8 种矿石）→ 土高炉烧 30s → 1 个 ingot。无中间产物、无副产物、无处理链深度，缺乏 GT:NH 的灵魂玩法。

本次加入 **3 阶段矿物处理链**，让玩家通过建设破碎机 → 洗矿槽 → 离心机 三级工厂，将 1 矿石最终产出 2 ingot（×2 效率奖励），并伴生 gravel/sand/稀有矿粉等副产物，深化电力+机器系统的经济与工程深度。

### 顺带修复的 2 个现有 bug
1. **itemPrice 缺 5 种 ingot 售价**：aluminum/tin/copper/gold_ingot/silver_ingot 当前返回 0，土高炉产物根本无法出售，新 dust→ingot 链会继承同一死胡同。
2. **findNearbyGenerator 2×2 邻接盲区**：当前只检查左上角单格曼哈顿距离，2×2 块漏掉右侧/下侧/角邻接位，玩家站某些位置按 E 打不开发电机面板。

---

## 设计决策（用户已确认）

| 维度 | 选择 |
|------|------|
| 范围 | 完整 3 阶段：Crusher + Washer + Centrifuge + 24 种新物品 + 副产物 + 3 蓝图 |
| UI | 混合：Crusher 4 槽面板（高频操作，镜像 FurnaceUI），Washer/Centrifuge 单槽简洁面板（镜像 GeneratorPanel 内联范式） |
| 产出倍率 | GT:NH 经典 ×2：1 ore → 2 crushed → 2 purified → 2 dust → 2 ingot |

### 关键架构决策
- **多机器实例**：用 `map<pair<int,int>, Machine>` 而非单实例（镜像 generators map L1853）
- **Crusher 4 槽电力**：per-slot `bool hasPowerThisTick[4]` 数组，`globalTick100ms` 逐槽 `powerDraw(4, pos)`，原子扣减保证抢电语义
- **Washer/Centrifuge 电力**：单 `bool hasPowerThisTick`，镜像 Lathe L505 范式
- **UI 焦点链**：所有按钮 `Container::Vertical({...})` 必须赋值给变量再作为 Renderer 第一个参数，禁止悬空 Container（已踩过 2 次坑）

---

## 实施任务（10 步）

### T1 — 物品体系与售价表扩展
**位置**：[Chemical-World.cpp L112-126](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L112-L126) `itemPrice()` 自由函数

**新增**：
- 修 5 种现有 ingot 售价：`aluminum=15, tin=18, copper=20, gold_ingot=60, silver_ingot=35`
- 24 种处理链主产物：
  - crushed_*（8 种，value=8，category="crushed"）
  - purified_*（8 种，value=12，category="purified"）
  - *_dust（8 种，value=18，category="dust"）
- 副产物：`gravel=1, gold_dust=30, copper_dust=20`（sand 已有）

### T2 — 新机器类定义
**位置**：在 [Lathe 类结束 L620](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L620) 之后、FurnaceUI L924 之前插入 3 个类

**Crusher 类**（镜像 BlastFurnace L176-408）：
- 4 槽 `Slot` + `SlotPhase {IDLE, STONING, MILLING, DONE_POUR}`
- 8 配方：`{oreName → crushedName, oreRequired=1, crushedAmount=2, byproduct="gravel", byproductAmount=1, durationMs=10000}`
- `bool hasPowerThisTick[4]` + `int frameIndex[4]`
- 12 帧动画：0-3 STONING（石块进料）/4-8 MILLING（齿轮旋转）/9-11 DONE_POUR（碎屑流出）

**OreWasher 类**（镜像 Lathe L415-620 单槽状态机）：
- `AnimState {Idle, Washing, Done}` + 单 `bool hasPowerThisTick`
- 8 配方：`{crushedName → purifiedName, byproduct="sand", byproductAmount=1, rareChance=30, rareByproduct="copper_dust"/"gold_dust"}`
- durationMs=5000

**Centrifuge 类**（同 Washer 结构）：
- durationMs=15000
- 8 配方：`{purifiedName → dustName, byproduct="rare_dust"/"gold_dust"}`
- 稀有副产物概率更高（50%）

### T3 — BlastFurnace 配方扩展（dust 路径）
**位置**：[BlastFurnace::recipes L208-217](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L208-L217) 追加 8 条

```cpp
{"Steel From Dust",  "hematite_dust",  "steel",       2, 1, 2, 30000},  // ×2 效率
{"Aluminum From Dust","bauxite_dust",  "aluminum",    2, 1, 2, 30000},
// ... 共 8 条，oreRequired=2, resultAmount=2
```

### T4 — ChemicalWorldGame 字段与 newGame
**位置**：[L1847-1854](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1847-L1854) 字段 + newGame L1999-2024

**新增字段**：
```cpp
map<pair<int,int>, Crusher> crushers;
map<pair<int,int>, OreWasher> washers;
map<pair<int,int>, Centrifuge> centrifuges;
bool crusher_blueprint_unlocked = false;
bool washer_blueprint_unlocked = false;
bool centrifuge_blueprint_unlocked = false;
```

**newGame** 重置这 6 个字段。

### T5 — 电力系统扩展
**位置**：
- [tickPowerGrid::isConductive L2741](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2741) 追加 `X/x/W/w/R/r`
- [tickPowerGrid 机器通电判定 L2760](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2760) 追加 `'X' || 'W' || 'R'`
- [globalTick100ms L2841-2859](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2841-L2859) 在 lathe.update 之后追加 3 个 map 遍历

**调度顺序**：`tickPowerGrid → furnace → lathe → crushers → washers → centrifuges`（先到先得，GT:NH 风格）

**Crusher 调度**（关键：per-slot powerDraw）：
```cpp
for (auto& kv : crushers) {
    auto& pos = kv.first;
    auto& cr = kv.second;
    for (int i = 0; i < 4; ++i) {
        bool active = (cr.getSlot(i).phase != Crusher::IDLE && cr.getSlot(i).phase != Crusher::DONE_POUR);
        cr.setSlotPower(i, active ? powerDraw(4, pos) : true);
    }
    cr.update(player, 100);
}
```

### T6 — place 函数 + BuildUI 扩展
**位置**：
- 新增 `placeCrusher/placeWasher/placeCentrifuge`（在 [placeGenerator L2783](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2783) 后）
- [openBuildUI::rebuildPlaceEntries L2889-2900](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2889-L2900) 追加 3 条菜单
- [tryPlace switch L2906-2935](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2906-L2935) 追加 case 4/5/6
- [infoRenderer L2974-2987](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2974-L2987) 追加 3 行蓝图状态

**字母分配**：
- Crusher = `X`/`x`，COLOR_YELLOW/DARK_YELLOW，150c
- Washer = `W`/`w`，COLOR_CYAN/DARK_CYAN，120c
- Centrifuge = `R`/`r`，COLOR_PURPLE/DARK_PURPLE，200c

### T7 — E 键交互 + findNearbyMachine 通用查找
**位置**：
- 新增 `findNearbyMachine(char type, px, py, outPos)`（2×2-aware，4 格遍历）在 [findNearbyGenerator L2828](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2828) 旁
- 顺手重写 `findNearbyGenerator` 调用 `findNearbyMachine('G', ...)` 修复盲区
- [processKey case 'e' L2616-2654](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2616-L2654) 在 generator 块之后追加 3 个机器查找

```cpp
bool findNearbyMachine(char type, int px, int py, pair<int,int>& outPos) const {
    if (currentArea != Area::Home) return false;
    for (auto& m : machineMeta) {
        if (m.type != type) continue;
        for (int dy = 0; dy < 2; ++dy) for (int dx = 0; dx < 2; ++dx) {
            if (abs(m.x + dx - px) + abs(m.y + dy - py) <= 1) {
                outPos = { m.x, m.y };
                return true;
            }
        }
    }
    return false;
}
```

### T8 — 三个 UI 函数
**位置**：在 [openGeneratorPanel L3132](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L3132) 之后追加

**CrusherUI（4 槽，独立类，镜像 FurnaceUI L924-1192）**：
- 4 槽 Radiobox + 配方 Menu + 4 进度条 + 12 帧 ASCII art
- 构造 `CrusherUI(Crusher&, PlayerData&, ScreenInteractive&)`
- 焦点链：`Container::Vertical({ slotTabs, recipeList, buttonLoad, buttonCancel, buttonCollect, buttonClose })`
- ticker thread 调 globalTick100ms，CatchEvent(Event::Custom) 仅刷新

**openWasherUI(x,y) / openCentrifugeUI(x,y)**（内联函数，镜像 openGeneratorPanel L3053-3132）：
- 配方 Radiobox + 单进度条 + LOAD/CLOSE 按钮
- `buttons = Container::Vertical({ buttonLoad, buttonClose })` 必须赋值给变量
- ticker thread 调 globalTick100ms

### T9 — 存档 v4 升级
**位置**：[saveGame L1885-1924](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1885-L1924) + [loadGame L1926-1997](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1926-L1997)

**v4 字节布局**：
```
CHEMICAL_WORLD_SAVE 4\n
<player 字段>\n
<inventory>\n
<tutorials>\n
<map>\n
[v4 新增段]
<crusher_bp> <washer_bp> <centrifuge_bp> <gen_bp> <wire_bp>\n   <- 5 蓝图一行
<globalEU>\n
<metaCount>\n
<MachineMeta>*metaCount: <x> <y> <type:char> <remainingBurnEU> <loadedCoal> <active:int>\n
```

**loadGame v3 兼容**：
- version==3 分支：3 新蓝图默认 false，读 gen/wire 2 字段
- version==4 分支：读 5 蓝图字段
- meta 读取后，type=='X'/'W'/'R' 时重建对应 map 实例（镜像 L1994 generators 重建）

### T10 — 地图图例 + BOM 修复 + 构建验证
**位置**：
- [renderArea 图例 L2523-2530](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2523-L2530) 追加 `X=Crusher W=Washer R=Centrifuge`
- 文件首 3 字节验证 `EF BB BF`（单 BOM），若双 BOM 需截断
- MSBuild Debug|x64 构建 EXIT=0

---

## 验证清单

### 构建
1. MSBuild EXIT=0，无 C4819 编码警告
2. 文件首字节 `EF BB BF`（单 BOM）
3. 文件行数 ~3400-3500

### 端到端流程
1. 新游戏 → 3 蓝图均 false，3 个机器 map 为空
2. TradeUI BUY → 购 Crusher BP(200c) → 解锁 + 扣币
3. BuildUI → 选 Crusher → PLACE → 地图出现 'X' 2×2 + machineMeta 追加 + crushers map 插入
4. 电线连接 发电机→Crusher → BFS 导通 → poweredMachines 含 crusher 坐标
5. CrusherUI → E 邻近 'X' → 4 槽 Radiobox + 8 配方 Menu + 12 帧 art
6. 电力抢电：2 台 Crusher 各 4 槽全开 EU=20 → 前 5 槽推进、第 6 槽暂停
7. WasherUI → LOAD crushed_hematite → 5s → purified_hematite + sand + 30% copper_dust
8. CentrifugeUI → LOAD purified_hematite → 15s → hematite_dust + 50% rare
9. dust→ingot 闭环：BlastFurnace "Steel From Dust" → 2 dust + 1 coal → 30s → 2 steel（验证 ×2）
10. ingot 可售：aluminum/tin/copper/gold_ingot/silver_ingot 显示价格并出售
11. 存档 v4 → 读首行 `CHEMICAL_WORLD_SAVE 4` → 5 蓝图 + globalEU + meta 含 X/W/R
12. v3 向后兼容 → 旧 v3 存档加载 → 3 新蓝图 false、gen/wire 正确、F/L/G meta 重建
13. v4 重载 → 3 蓝图仍 owned、3 台机器在原位、map 实例重建
14. 焦点链无悬空 → Tab 在各按钮间循环无消失
15. 2×2 邻接盲区修复 → 站 Crusher 右下角 (x+2,y+2) → E 仍能打开面板

---

## 关键文件
- [Chemical-World.cpp](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp)（单文件，所有修改集中在此）
- [Chemical-World.vcxproj](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.vcxproj)（构建配置，确认 C++20 + FTXUI 6.1.9 路径未变）

## 复用范式
- 2×2 机器放置：[placeGenerator L2783-2800](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2783-L2800)
- 4 槽 UI：[FurnaceUI L924-1192](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L924-L1192)
- 单槽简洁 UI：[openGeneratorPanel L3053-3132](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L3053-L3132)
- 状态机+动画：[Lathe L415-620](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L415-L620)
- 电力调度：[globalTick100ms L2841-2859](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2841-L2859)
- 存档 v3 范式：[saveGame/loadGame L1885-1997](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1885-L1997)
