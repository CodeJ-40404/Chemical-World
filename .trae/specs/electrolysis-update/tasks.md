# 电解更新 - 实施计划

## Task 1：扩展 itemPrice 与化学工具函数
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: None
- **Description**:
  - 在 itemPrice 增加 12 个新名：9 元素（hydrogen 30/oxygen 25/carbon 20/nitrogen 35/sulfur 40/chlorine 50/sodium 60/magnesium 45/silicon 55）、3 化合物（salt 15/sodium_hydroxide 120/copper_sulfate 180/iron_sulfide 90）。
  - 新增文件级工具：`isElement(name)`（9 新元素 + 6 金属锭名共 15）、`isCompound(name)`（salt/sodium_hydroxide/copper_sulfate/iron_sulfide）。
- **Acceptance Criteria Addressed**: AC-1, AC-5
- **Test Requirements**:
  - `rule` TR-1.1：grep itemPrice 确认 12 个新名均存在且价格 > 0
  - `rule` TR-1.2：isElement("hydrogen")=true、isElement("water")=false、isElement("copper")=true；isCompound("salt")=true

## Task 2：背包新增 ELEMENTS 与 COMPOUNDS 分类
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 1
- **Description**:
  - openBackpack cats 表在 misc 前插入 `{ "element", "ELEMENTS", Color::CyanLight }` 与 `{ "compound", "COMPOUNDS", Color::GreenLight }`。
  - 元素物品产出时 category="element"；新化合物 category="compound"；water 保持原分类（不强制归类，但 isCompound 仍识别）。
- **Acceptance Criteria Addressed**: AC-1
- **Test Requirements**:
  - `rule` TR-2.1：背包界面出现 ELEMENTS / COMPOUNDS 两个标签页

## Task 3：Electrolyzer 电解机类
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 1
- **Description**:
  - 仿 OreWasher 单槽结构：AnimState(Idle/Electrolyzing/Done)、Recipe（name/inputName/inputs 支持 1-2 种/ outputs vector<name,amount>/durationMs 8000）。
  - 4 配方：water→{hydrogen,2}+{oxygen,1}；bauxite_dust→{aluminum,2}+{oxygen,3}；salt→{sodium,1}+{chlorine,1}；salt+water→{chlorine,1}+{hydrogen,1}+{sodium_hydroxide,1}。
  - collect 发放多产物；canLoad 检查所有 input 数量。
- **Acceptance Criteria Addressed**: AC-2, AC-3
- **Test Requirements**:
  - `rule` TR-3.1：4 条配方 inputs/outputs 与 FR-4 一致
  - `rule` TR-3.2：hasPowerThisTick=false 时 update 不推进 accumulatedMs

## Task 4：ChemistryBench 化合台类
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 1
- **Description**:
  - 仿 Lathe 多输入配方：Recipe（name/inputs vector<name,amount>/outputName/outputAmount），不耗电。
  - 5 配方：2H+O→water；Na+Cl→salt；Na+O+H→sodium_hydroxide；Cu+S+4O→copper_sulfate；Fe+S→iron_sulfide。
  - canCraft 检查输入；craft 扣除输入、addItem 输出。
- **Acceptance Criteria Addressed**: AC-4
- **Test Requirements**:
  - `rule` TR-4.1：5 条配方输入消耗与产出与 FR-6 一致

## Task 5：Game 状态字段与 newGame
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 3, Task 4
- **Description**:
  - 成员：`electrolyzer_blueprint_unlocked`、`chembench_blueprint_unlocked`、`map<pair<int,int>, Electrolyzer> electrolyzers`、`map<pair<int,int>, ChemistryBench> chemBenches`。
  - newGame 重置两个蓝图 false、清空两个 map。
- **Acceptance Criteria Addressed**: AC-7
- **Test Requirements**:
  - `rule` TR-5.1：newGame 后两蓝图 false、两 map 为空

## Task 6：放置函数与 rehydrate
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 5
- **Description**:
  - placeElectrolyzer(x,y)：蓝图门禁、250c、2×2 空地、stamp Z/z（COLOR_CYAN/COLOR_DARK_CYAN）、map 插入、machineMeta push。
  - placeChemBench(x,y)：蓝图门禁、120c、2×2 空地、stamp K/k（COLOR_GREEN/COLOR_DARK_GREEN）。
  - rehydrateMachineTiles 类型表加 {Z,z,...} 与 {K,k,...}。
- **Acceptance Criteria Addressed**: AC-7
- **Test Requirements**:
  - `rule` TR-6.1：两机器 tile 字符 Z/z、K/k 不与现有矿/装饰字符冲突

## Task 7：BuildUI 列表、tryPlace、Home 图例
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 6
- **Description**:
  - placeEntries 在 Gem Sorter 后加两项：Electrolyzer (2x2,250c)、Chemistry Bench (2x2,120c)；Flower/Grass case 顺延。
  - tryPlace switch 加 case 6/7（Electrolyzer/ChemBench），原 Flower/Grass 变 case 8/9。
  - BuildUI 状态面板加两行 BP 显示；Home 图例加 Z=Electrolyzer、K=ChemBench。
- **Acceptance Criteria Addressed**: AC-7
- **Test Requirements**:
  - `rule` TR-7.1：B 菜单顺序为 Gen/Wire/Crusher/Washer/Centrifuge/Sorter/Electrolyzer/ChemBench/Flower/Grass；case 序号与列表一致

## Task 8：E 键分发与两台机器 UI
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 6, Task 7
- **Description**:
  - E 键 findNearbyMachine('Z')→openElectrolyzerUI；('K')→openChemBenchUI。
  - openElectrolyzerUI 仿 openSorterUI（Radiobox 配方 + LOAD/COLLECT/CLOSE + 100ms ticker），显示 10 EU、多产物列表、上次结果。
  - openChemBenchUI 仿车床/机器 UI（Radiobox 配方 + CRAFT/CLOSE），不耗电。
- **Acceptance Criteria Addressed**: AC-2, AC-4, AC-7
- **Test Requirements**:
  - `rule` TR-8.1：靠近 Z 按 E 打开电解机 UI；靠近 K 按 E 打开化合台 UI

## Task 9：globalTick 电力循环
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 5
- **Description**:
  - globalTick100ms 加 electrolyzers 循环：processing 时 `powerDraw(10, pos)`，setHasPowerThisTick，update。
  - 化合台不耗电，不进 globalTick。
- **Acceptance Criteria Addressed**: AC-3, AC-7
- **Test Requirements**:
  - `rule` TR-9.1：电解机运行中每 tick 扣 10 EU；无电时 setHasPowerThisTick(false)

## Task 10：TradeUI 集成
- **Status**: `completed`
- **Completion Evidence**: 实现完成；R1 独立审查通过（仅 3 项发现 I-1/I-2/I-3，已修复）；修复后 MSBuild EXIT=0
- **Priority**: high
- **Depends On**: Task 1, Task 5
- **Description**:
  - TradeUI 构造加 2 个 bool&（electrolyzerBP、chembenchBP）。
  - rebuildBuy BP 表加 2 项：Electrolyzer Blueprint 300c、Chemistry Bench Blueprint 180c。
  - BUY 原材料加 salt（15c×3=45c 买入）；元素不进 BUY。
  - 蓝图购买分发加 electrolyzer/chembench 分支。
  - trade() 构造调用补 2 参数。
- **Acceptance Criteria Addressed**: AC-5, AC-7
- **Test Requirements**:
  - `rule` TR-10.1：BUY tab 可见 salt、Electrolyzer BP(300)、ChemBench BP(180)；元素不在 BUY

## Task 11：存档 v7
- **Status**: `completed`
- **Priority**: high
- **Depends On**: Task 5
- **Description**:
  - saveGame header "CHEMICAL_WORLD_SAVE 7"；蓝图行写 8 int（末尾 electrolyzer、chembench）。
  - loadGame 接受 v2..v7；v≤6 蓝图默认 false，不读最后 2 int；v7 多读 2 int。
  - machineMeta 通用序列化天然支持 Z/K。
- **Acceptance Criteria Addressed**: AC-6
- **Test Requirements**:
  - `rule` TR-11.1：v6 存档加载后两蓝图 false；保存变 v7；v7 再加载两蓝图状态正确
- **Completion Evidence**: save header v7、蓝图行 8 int、load 接受 v2..v7、v7 分支读最后 2 int；构建 EXIT=0

## Task 12：BOM 检查与构建
- **Status**: `completed`
- **Priority**: high
- **Depends On**: Task 1-11
- **Description**:
  - 验证文件仅 1 个 UTF-8 BOM；MSBuild Debug x64 构建 EXIT=0。
- **Acceptance Criteria Addressed**: AC-8
- **Test Requirements**:
  - `rule` TR-12.1：BOM count == 1
  - `rule` TR-12.2：MSBuild EXIT CODE == 0，生成 Chemical-World.exe
- **Completion Evidence**: BOM count=1；MSBuild EXIT CODE=0，Chemical-World.exe 生成

## Issue I-1：电解机/筛选机未接入电网（审查 R1 发现 #1）
- **Status**: `completed`
- **Priority**: high
- **Depends On**: None
- **Discovered By**: Review R1
- **Description**:
  - tickPowerGrid 的 isConductive 与 poweredMachines 循环未包含 'Z'（电解机），且遗留 'S'（筛选机）同样遗漏，导致 powerDraw 永远返回 false，机器无法运行。
- **Acceptance Criteria Addressed**: AC-3, AC-7
- **Test Requirements**:
  - `rule` TR-I-1.1：isConductive 含 S/s/Z/z/K/k；poweredMachines 循环含 'S'、'Z'
- **Completion Evidence**:
  - isConductive 加 `d == 'S' || d == 's' || d == 'Z' || d == 'z' || d == 'K' || d == 'k'`；poweredMachines 条件加 `m.type == 'S' || m.type == 'Z'`；构建 EXIT=0

## Issue I-2：water 分类无背包标签页（审查 R1 发现 #2）
- **Status**: `completed`
- **Priority**: medium
- **Depends On**: None
- **Discovered By**: Review R1
- **Description**:
  - water 用 "basic" 分类但背包 cats 无此标签，导致初始赠送与化合产出的水在背包不可见。
- **Acceptance Criteria Addressed**: AC-1
- **Test Requirements**:
  - `rule` TR-I-2.1：背包 cats 含 "basic" 键
- **Completion Evidence**:
  - cats 表新增 `{ "basic", "BASIC / RAW", Color::BlueLight }`；构建 EXIT=0

## Issue I-3：copper_sulfate 化合亏本（审查 R1 发现 #3）
- **Status**: `completed`
- **Priority**: medium
- **Depends On**: None
- **Discovered By**: Review R1
- **Description**:
  - copper(48)+sulfur(40)+4×oxygen(100)=188c 投入，copper_sulfate 仅售 180c，净亏 8c。
- **Acceptance Criteria Addressed**: AC-9
- **Test Requirements**:
  - `rule` TR-I-3.1：copper_sulfate 售价 > 投入原料成本（188c）
- **Completion Evidence**:
  - itemPrice copper_sulfate 180→220c（毛利 32c）；构建 EXIT=0

