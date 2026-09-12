# 电解更新（Electrolysis Update）- 产品需求文档

## Overview
- **Summary**：在现有矿物处理链末端新增"化学"层——引入约 15 种元素（9 种新元素物品 + 6 种现有金属锭作为元素）、电解机（化合物→元素）与化合台（元素→化合物），形成"采矿 → 电解 → 元素 → 化合 → 高价值化合物"的化学闭环。
- **Purpose**：继宝石筛选机的"惊喜产出"后，加入有真实化学依据的深加工层，用元素周期表概念提升深度，让电力系统（电解耗电高）有更多用武之地。
- **Target Users**：已打通破碎机/洗矿/离心/冶炼主链、寻求更高附加值加工路线的玩家。

## Goals
- 引入 15 种元素概念，覆盖与现有矿石相关的 H/O/C/N/S/Cl/Na/Mg/Si（9 种新物品）及 Fe/Cu/Al/Sn/Ag/Au（复用现有锭）。
- 新增电解机（Electrolyzer）：按经典电解反应分解化合物为元素，高耗电（10 EU/tick），与离心机并列的深加工终端。
- 新增化合台（Chemistry Bench）：把元素合成为化合物（水、盐、硫酸铜、氢氧化钠等），形成可逆化学循环。
- 元素/化合物均可交易；部分元素（H/Cl/Na）只能通过电解获得，有稀缺价值。
- 与现有机器/存档/交易系统无缝集成，存档升 v7 并兼容 v2-v6。

## Non-Goals
- 不引入完整 118 元素周期表（仅 ~15 种相关元素）。
- 不重构现有冶炼/加工链（炉子炼金属锭不变；电解是平行/补充路径，不取代）。
- 不引入化学反应配平系统（按整数份配料，不强制摩尔比，简化可玩性）。
- 不做液体/气体状态管理（元素和化合物都作为普通背包物品）。

## Background & Context
- 现有主链：原矿 → 破碎 → 洗矿 → 离心（矿粉）→ 冶炼（锭）→ 车床（钢零件）；宝石支线独立。
- 已存在 `water`（2c）、`bauxite_dust`、`copper`、`aluminum` 等可作为电解输入的化合物/元素。
- 背包分类（openBackpack cats）：ore / fuel / material / product / gem / blueprint / misc —— 需新增 `element` 与 `compound` 两个分类。
- 机器模板：单槽机（Washer/Centrifuge/Sorter）用 AnimState + Recipe + powerDraw；化合台参照 Lathe（多输入配方）。
- 存档当前 v6，蓝图行 6 int；升 v7 加 electrolyzer + chemistry_bench 两个蓝图。

## Functional Requirements
- **FR-1（元素物品）**：新增 9 种元素物品 `hydrogen, oxygen, carbon, nitrogen, sulfur, chlorine, sodium, magnesium, silicon`；现有 `iron_ingot/copper/aluminum/tin/silver_ingot/gold_ingot` 视为铁/铜/铝/锡/银/金元素，不新增重名物品。元素在背包 `ELEMENTS` 分类。
- **FR-2（化合物物品）**：新增 `salt`、`sodium_hydroxide`、`copper_sulfate` 三种化合物；`water`、`bauxite_dust` 复用现有。化合物在背包 `COMPOUNDS` 分类（water 保持原分类可被两处识别）。
- **FR-3（电解机）**：2×2 机器，地图字符 `Z`/`z`，蓝/青色，10 EU/tick，8 秒/次；蓝图 300c（T 购买），放置费 250c；单槽，配方列表，吃化合物产出元素（多产物）。
- **FR-4（电解配方）**：
  - 水 → 2 hydrogen + 1 oxygen
  - bauxite_dust → 2 aluminum + 3 oxygen
  - salt → 1 sodium + 1 chlorine
  - salt + 1 water → 1 chlorine + 1 hydrogen + 1 sodium_hydroxide（氯碱）
- **FR-5（化合台）**：2×2 机器，地图字符 `K`/`k`，绿色，不耗电（手动配方），蓝图 180c，放置费 120c；参照 Lathe，配方消耗元素产出化合物。
- **FR-6（化合配方）**：
  - 2 hydrogen + 1 oxygen → water
  - 1 sodium + 1 chlorine → salt
  - 1 sodium + 1 oxygen + 1 hydrogen → sodium_hydroxide
  - 1 copper + 1 sulfur + 4 oxygen → copper_sulfate
  - 1 iron + 1 sulfur → iron_sulfide（额外化合物，可卖）
- **FR-7（交易）**：新元素/化合物进入 itemPrice；元素/化合物在 T 的 SELL 正常出售；BUY 不售元素（电解独占），但 `salt` 可买（作为电解入口）。
- **FR-8（背包）**：新增 `ELEMENTS` 与 `COMPOUNDS` 两个分类标签。
- **FR-9（存档）**：版本升 v7，蓝图行写 8 int（末尾 electrolyzer、chemistry_bench）；v≤6 默认 false；机器实例 find-or-insert 不存档（同现有机器）。
- **FR-10（图例）**：Home 图例加 Z（电解机）、K（化合台）。
- **FR-11（电源）**：电解机在 globalTick 中按 `powerDraw(10, pos)` 扣电，断电暂停（同 Washer 语义）。

## Non-Functional Requirements
- **NFR-1**：单文件实现，仅改 `Chemical-World.cpp`。
- **NFR-2**：FTXUI 组件必须同时挂进 Container 且在 Renderer 中 Render，避免悬空按钮。
- **NFR-3**：存档 BOM 唯一（单 UTF-8 BOM）。
- **NFR-4**：MSBuild Debug x64 EXIT=0。

## Constraints
- **Technical**：FTXUI 6.1.9 无 Element 版 Maybe；无 Color::DarkYellow（用 YellowLight）；无 Event::Space。
- **Business**：电解机/化合台为后期高价值路线，蓝图总价不低（300c + 180c），需玩家先有经济基础。
- **Dependencies**：依赖现有电力系统（EU pool）、机器 meta/序列化、TradeUI、背包分类。

## Assumptions
- 玩家已理解现有 2×2 机器放置（B 菜单）与 E 键交互模式。
- `bauxite_dust` 可直接作为氧化铝来源电解出铝（不再加 alumina 中间品）。
- `salt` 作为可买原料提供氯碱入口；`water` 玩家自带或可买。

## Acceptance Criteria

### AC-1：元素与化合物物品可获得并分类
- **Type**: `rule`
- **Given**：玩家拥有电解机与化合台
- **When**：电解水得到 hydrogen/oxygen，化合钠+氯得到 salt
- **Then**：这些物品出现在背包 ELEMENTS / COMPOUNDS 分类下，名称正确
- **Pass Condition**：9 种新元素 + 3 种新化合物均能通过机器产出并在对应分类可见
- **Evidence**：手测背包分类显示；代码 grep 确认 itemPrice 含全部 12 个新名

### AC-2：电解机按配方产出元素
- **Type**: `rule`
- **Given**：电解机已通电、装入 1 water
- **When**：等待 8 秒进度完成后 COLLECT
- **Then**：背包获得 2 hydrogen + 1 oxygen，机器回到 IDLE
- **Pass Condition**：4 条电解配方均产出正确元素种类与数量
- **Evidence**：手测 4 条配方各一次，核对产物；代码中 recipes 表核对

### AC-3：电解机断电暂停
- **Type**: `rule`
- **Given**：电解机正在运行中切断电源
- **When**：电力恢复
- **Then**：运行期间进度不推进；恢复后续接上次进度
- **Pass Condition**：`setHasPowerThisTick(false)` 时 update 不推进 accumulatedMs
- **Evidence**：代码逻辑审查 + 手测断电观察进度条

### AC-4：化合台按配方合成化合物
- **Type**: `rule`
- **Given**：化合台放入 2 hydrogen + 1 oxygen
- **When**：执行合成
- **Then**：消耗原料，产出 1 water
- **Pass Condition**：5 条化合配方均消耗正确输入、产出正确化合物
- **Evidence**：手测 5 条配方；代码 recipes 表核对

### AC-5：交易系统支持新物品
- **Type**: `rule`
- **Given**：玩家背包有 hydrogen 与 salt
- **When**：打开 T → SELL
- **Then**：hydrogen 显示售价可卖；salt 可买可卖
- **Pass Condition**：itemPrice 对所有新物品返回正值；BUY tab 可买 salt；元素不在 BUY tab
- **Evidence**：grep itemPrice；手测 T 界面

### AC-6：存档兼容与升级
- **Type**: `rule`
- **Given**：一个 v6 存档
- **When**：加载并保存
- **Then**：v6 正常加载（电解/化合蓝图默认 false），保存后变 v7，再次加载正常
- **Pass Condition**：loadGame 接受 v2..v7；v7 蓝图行 8 int；v≤6 不读最后 2 int
- **Evidence**：手测加载旧档→保存→再加载；代码版本分支审查

### AC-7：新机器可放置、交互、通电
- **Type**: `rule`
- **Given**：已购买电解机与化合台蓝图
- **When**：B 菜单放置、接电线、按 E
- **Then**：机器出现在地图（Z/z、K/k），E 打开 UI，通电后可运行
- **Pass Condition**：placeElectrolyzer/placeChemBench 扣费并 stamp tile；E 键分发命中；globalTick 扣电
- **Evidence**：手测放置→E→接电→运行全链路

### AC-8：构建无错误
- **Type**: `rule`
- **Given**：完成所有代码改动
- **When**：MSBuild Debug x64
- **Then**：EXIT CODE = 0，无新增编译错误
- **Pass Condition**：构建输出 `Chemical-World.exe` 成功
- **Evidence**：MSBuild 命令输出

### AC-9：化学系统经济与可玩性平衡
- **Type**: `rubric`
- **Dimension**：元素/化合物定价与流水线收益合理性
- **Scale**: 1-5
- **Anchors**: 1 = 元素售价低于原料成本（亏本无人玩）；3 = 微利或与冶炼相当；5 = 电解/化合有明确高于直接卖原料的利润，但不过度碾压现有路线
- **Pass Threshold**: >= 4
- **Evidence**：核对 itemPrice 中元素价 > 对应原料电解消耗价；手测一轮水→H/O→化合 收益

## Open Questions
- [ ] iron_sulfide 是否作为额外化合配方加入？（默认加入，丰富化合台内容；若觉得冗余可砍）
- [ ] salt 是否需要在 OCEAN 生物群系作为矿物自然产出？（当前仅可买；若要"采盐"体验再加）
