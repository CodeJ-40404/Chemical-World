# 电力系统实现计划

## Context

游戏 [Chemical-World.cpp](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp)（~2215 行）已有 Furnace/Lathe 两台机器 + FTXUI 全屏子界面模式 + 版本 2 存档。本轮引入完整电力系统：玩家默认拥有家园，可在 Home 按 **B** 进入 FTXUI 子图缩放建造界面，铺设水管/电线（多层可堆叠）、放置水泵（邻水被动抽水）和热力发电机（灌水+煤产电），通过真实电网 BFS 连通性计算供电；F/L 在无电时拒动。新存档版本 3 向后兼容 v2。

**用户已确认的核心决策：** 多层 Tile（terrain+pipe+wire 可堆叠）、真实电网（BFS 连通分量发电 vs 耗电）、全做（河流湖泊+B键建造+发电机+F/L接电力）、FTXUI 子地图放大界面。

**引导期设计**：`hasPower()` 在 `powerNet==nullptr` 或 `generators.empty()` 时返回 true——玩家未建发电机前 F/L 仍可正常运行，避免冷启动死锁（没电就没法炼钢，没钢就没法建发电机）。一旦放下首台发电机，门控立即生效。

---

## 数据模型

### Tile 扩展（[L575-584](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L575)）
采用「Tile 内新增层字段」方案（渲染/查找/存档最简单，不破坏 `tiles[y][x]` 索引）：
```cpp
struct Tile {
    char display; string name; string description; bool passable; int color;
    string mineral; int richness = 0; int hits = 0;
    char pipe = 0;   // 0=无管道, 'p'=有管道（可走过）
    char wire = 0;   // 0=无电线, 'w'=有电线（可走过）
};
```
机器 'G'(发电机)/'K'(水泵) 非可走过（同 'F'/'L'）；管道/电线可走过；水 '~' 在 Home 不可走过（需泵邻接抽水）。

### Generator / Pump 结构（PowerNetwork 内）
```cpp
struct Generator { int x,y; int waterLevel=0, coalLevel=0; int maxOutput=100; int tickAccumMs=0; };
struct Pump { int x,y; };  // 本轮被动，无内部状态
```
发电机：water/coal 任一为 0 → 输出 0；两者 >0 → 输出 maxOutput。耗率：每 3s -1 水，每 5s -1 煤。

### PowerNetwork 类（声明位置：紧前 [L1462](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1462) ChemicalWorldGame）
```cpp
class PowerNetwork {
    GameMap* map=nullptr;
    vector<Generator> generators; vector<Pump> pumps;
    map<pair<int,int>,bool> poweredMap;      // 消费者(x,y)->通电
    set<pair<int,int>> wateredPipeCells;     // 有水的管道格
public:
    void bind(GameMap& m);
    bool hasPower(int x,int y) const;  // generators空 或 poweredMap[x,y]=true
    bool hasWater(int x,int y) const; // wateredPipeCells 含 (x,y)
    void recompute();                 // 双 BFS（水网+电网）
    void tick(PlayerData&, int ms=100);
    void placeGenerator(int x,int y); void placePump(int x,int y);
    void removeMachine(int x,int y); void clear();
    Generator* generatorAt(int x,int y);
    void save(ostream&) const; bool load(istream&, int version);
};
```

### F/L 注入方式（关键设计）
给 `BlastFurnace` 和 `Lathe` 各加 `PowerNetwork* powerNet=nullptr; int gx=0,gy=0;` 成员 + `setPowerNet(PowerNetwork*, int x, int y)` setter。canLoad 顶部加：
```cpp
if (powerNet && !powerNet->hasPower(gx, gy)) return false;
```
- 不改 canLoad 签名（FurnaceUI/LatheUI 调用方不用改）
- null powerNet = 引导期免费
- 机器自带坐标，由 ChemicalWorldGame 在 newGame/loadGame/travelTo 后 `furnace.setPowerNet(&powerNet, 5,5); lathe.setPowerNet(&powerNet, 5,8);` 设置

---

## PowerNetwork 算法（BFS 伪码）

### recompute() — 先水后电，仅 Home 区运行（Wasteland/Cave 直接 return）
- **水网 BFS**：对所有 `tile.pipe!=0` 的格做连通分量；分量有水当且仅当某管道格四邻含 '~' 或格上有 'K' 且泵邻水；有水的分量所有格加入 `wateredPipeCells`。
- **电网 BFS**：对所有 `tile.wire!=0` 的格做连通分量；分量内累加 `genSum`（'G' 且 water>0 && coal>0 && 该格在 wateredPipeCells 才计入 maxOutput）与 `demandSum`（'F'=50, 'L'=30）；`powered = genSum>=demandSum`，分量内所有消费者写入 `poweredMap`。

### tick(player, 100ms)
推进每台 generator 的 tickAccumMs，按耗率扣 water/coal；调 recompute()。

---

## 存档/读档（版本 3，向后兼容 v2）

- 头部升级 `CHEMICAL_WORLD_SAVE 3`（[L1499](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1499)）。
- `loadGame` 接受 `version==2 || version==3`（[L1517](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1517)）。
- `GameMap::save`（[L723-732](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L723)）每 tile 写 **7 字段**：`display mineral richness hits color pipe wire passable`（pipe/wire 用 `(int)(unsigned char)`，passable 用 0/1 显式持久化以区分 Home 水 vs Wasteland 盐滩）。
- `GameMap::load(istream&, int version)`：v2 读 5 字段 + 旧 passable 启发式（pipe=0/wire=0）；v3 读 7 字段直接用 passable。
- `saveGame`（[L1509](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1509)）在 `gameMap.save` 后追加 `powerNet.save(output)`：
  ```
  POWER_NET <genCount> <pumpCount>
  <x> <y> <waterLevel> <coalLevel>   // 每 generator
  <x> <y>                            // 每 pump
  ```
- `loadGame`（[L1540](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1540) 后）调 `powerNet.bind(gameMap); powerNet.load(input, version); powerNet.recompute();`。v2 无此段→`powerNet.clear()`；v3 用 `input.peek()=='P'` 判段存在，缺段则 `input.clear()` fail-soft。

---

## BuildUI（B 键子图缩放）

### 进入：`openBuildUI()`（紧随 [L1912](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1912) openLatheUI）
镜像 openFurnaceUI：隐光标→Fullscreen→构造 BuildUI→ticker 线程 100ms PostEvent（推进 generator tick + 刷新）→Loop→join→复光标→cls。

### 视图
- 以玩家为中心 11×11 视图（边界裁剪，仿 [renderMap L1926-1929](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1926)）。
- 每 tile 渲染 **3 宽×2 高** 字符块：顶行左='p'/'.'（管道层）右='w'/'.'（电线层）；底行=地形/机器 display，光标格用 `[]` 包围。
- 右侧 Radiobox 选模式：`[Pipe][Wire][Generator][Pump][Remove][Exit]`。
- CatchEvent 拦方向键移光标（cx,cy 限地图内）；Enter/Space 触发 placeAt。

### 放置规则（space-check）
- **Pipe/Wire**：免费，可叠在任何格（设 tile.pipe='p' 或 tile.wire='w'），recompute。
- **Generator**：仅可放 passable 空地；耗 5 steel；存 displacedTerrain 还原；tile 置 'G'(COLOR_YELLOW) 不可走。
- **Pump**：仅可放 passable 空地且四邻含 '~'；耗 3 steel；tile 置 'K'(COLOR_CYAN) 不可走。
- **Remove**：'G'/'K' 还原 displacedTerrain（不退钢，防刷资源）；管道/电线清 0；recompute。
- **光标在 'G' 上按 Enter（selMode=Generator）**：不重复放置，改为打开 GeneratorUI。

### 费用（计划自定，可调）
管道/电线免费；发电机 5 钢；水泵 3 钢。给 steel 一个去处，又不过度限制首版。

---

## GeneratorUI（发电机面板）

类 `GeneratorUI`（紧随 LatheUI）：构造接 `PowerNetwork&, PlayerData&, Generator&, ScreenInteractive&`。
- 数值条用**手画 `[#...]` 风格**（仿 FurnaceUI progressBar，不依赖 FTXUI gauge API）。
- 三条：WATER（蓝）、COAL（黄）、POWER（绿，effectiveOutput/maxOutput）。
- 按钮：`ADD WATER`（耗 water×1, +25）/ `ADD COAL`（耗 coal×1, +25）/ `CLOSE`。
- CatchEvent(Event::Custom) 每 100ms 调 `powerNet.tick(player)` 推进消耗 + recompute，刷新数值条。

---

## 河流湖泊（[L640](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L640) 后）

Home generate 中雕刻：
- 湖泊：~(40-45, 30-35) 一团 '~'(COLOR_BLUE, passable=false)。
- 河流：从湖蜿蜒到地图边的 '~' 序列。
- 远离 F(5,5)/L(5,8)/car(12,5)/spawn(8,5)。

---

## 集成点

| # | 位置 | 改动 |
|---|---|---|
| 1 | [L575-584](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L575) | Tile 加 `pipe`/`wire` 字段 |
| 2 | [L723-752](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L723) | save/load 改 7 字段 + version 参数 |
| 3 | [L1499](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1499)/[L1517](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1517) | 版本 3 + 接受 v2/v3 |
| 4 | [L1509](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1509)/[L1540](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1540) 后 | saveGame 追加 powerNet.save；loadGame 加 powerNet.load + recompute |
| 5 | 紧前 [L1462](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1462) | PowerNetwork + Generator/Pump + BuildUI + GeneratorUI 类声明 |
| 6 | [L1464-1473](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1464) | 加 `PowerNetwork powerNet;` 成员 |
| 7 | [L1543-1558](file:///d:/d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1543) newGame / [L1813-1840](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1813) travelTo / loadGame 末 | `powerNet.clear(); powerNet.bind(gameMap); furnace.setPowerNet(&powerNet,5,5); lathe.setPowerNet(&powerNet,5,8); powerNet.recompute();` |
| 8 | [L640](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L640) 后 | 雕刻河流湖泊 |
| 9 | [L229-232](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L229) BlastFurnace::canLoad + [L451-454](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L451) Lathe::canLoad | 加 `powerNet` 指针成员 + setPowerNet + canLoad 顶部 hasPower 检查 |
| 10 | [L824-834](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L824) FurnaceUI buttonLoad / [L1089-1098](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1089) LatheUI buttonLoad | 欠压时 statusMessage="No power!" |
| 11 | 紧随 [L1912](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1912) | openBuildUI + openGeneratorUI |
| 12 | [L2096](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2096) 附近 | `case 'b':case 'B': if(Home) openBuildUI(); else message="Build only at home.";` |
| 13 | [L1965](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1965) legend / [L2038](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2038) help / [L2161](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L2161) help bar | 加 `G=Generator K=Pump ~=Water B=Build` 与 `[B Build]` |
| 14 | openFurnaceUI/openLatheUI 入口 | 各调 `powerNet.recompute()` 保证通电状态最新 |

---

## 实现顺序（基础先行，每步编译过）

1. **基础**：Tile 扩展 + GameMap save/load 7 字段 + 版本 3 + loadGame 接受 v2/v3
2. **PowerNetwork 骨架**：类 + bind + recompute 双 BFS + hasPower/hasWater
3. **成员接入**：powerNet 成员 + newGame/travelTo/loadGame 的 bind/setPowerNet/recompute
4. **河流湖泊** + legend
5. **BuildUI + openBuildUI + B 键**
6. **GeneratorUI + openGeneratorUI + tick 推进**
7. **F/L 门控**：canLoad + UI 提示

---

## 验证

1. **编译**：vcxproj x64 Debug，0 error。
2. **冷启动**：NEW GAME → F(5,5)/L(5,8) 仍可用（generators.empty()，hasPower=true）。
3. **河流湖泊**：Home 右下见 '~'（蓝），走不上去。
4. **建造**：B → 11×11 子图 → 选 Pump 放水边（-3钢）→ 选 Pipe 铺到 (5,5) → 选 Generator 放空地（-5钢）→ 选 Wire 连发电机到 F/L。
5. **发电**：子图内光标移到 'G' 按 Enter → GeneratorUI → ADD WATER/COAL → Power 条满。
6. **F 运行**：回主图 E 临 F → LOAD 成功（hasPower=true）。
7. **欠压拒动**：BuildUI Remove 拆发电机到 F 间一根电线 → recompute → F 欠压 → E 临 F → buttonLoad "No power!"。
8. **断水跳闸**：GeneratorUI 等水耗到 0 → effectiveOutput=0 → F 欠压。
9. **持久化**：存档→读档→'~'/'G'/'K'/管道/电线/发电机水位煤位全恢复。
10. **v2 兼容**：现有 v2 autosave 读档不报错，无管道电线，F/L 因 generators.empty() 可用。

---

## 风险与备注

- **规模**：约 800-1000 新行，单文件迄今最大变更。严格按实现顺序，每步编译。
- **发电机 tick 不在主循环推进**：首版只在 GeneratorUI/BuildUI 打开时 tick（UI ticker 线程）。关闭 UI 时 generator 不耗煤——简化首版。**缓解**：openFurnaceUI/openLatheUI 入口调 recompute()（只重算网络不 tick）保证状态最新。
- **BFS 成本**：Home 60×50=3000 格，每次 recompute 双 BFS O(3000)，放置时调一次可接受。recompute 入口断言 `if(map->getArea()!=Home) return;` 跳过非 Home。
- **存档兼容**：v3 读 v2（pipe=0/wire=0，passable 走启发式）；v2 存档无 Home 水，行为不变。
- **拆除不退钢材**：防刷资源。
- **字符冲突**：管道 'p'/电线 'w' 小写，不与 mineral 大写字符冲突（'P'=Malachite, 'G'=Gold/发电机）。子图用小写标记层，主图机器大写。
- **EOF/流状态**：powerNet.load 缺段需 `input.clear()` 恢复流状态。
- **tutorialCount 守卫**（[L1531](file:///d:/c++/repos/Chemical-World/Chemical-World/Chemical-World.cpp#L1531)）：不动 tutorials vector，避免触发守卫报错。
- **BuildUI 越界**：玩家在角时 11×11 视图裁剪，仿 renderMap 的 `viewX=max(0,...)` 模式。
