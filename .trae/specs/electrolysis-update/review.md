# 电解更新 - 独立审查

- [ ] CP-R1: 13 个新物品（9 元素 + 4 化合物）在 itemPrice 中存在且价格 > 0
  - **Type**: `rule`
  - **Covers**: AC-1, AC-5
  - **Evidence**: Pending

- [ ] CP-R2: 4 条电解配方输入/输出与 FR-4 一致
  - **Type**: `rule`
  - **Covers**: AC-2
  - **Evidence**: Pending

- [ ] CP-R3: 5 条化合配方输入/输出与 FR-6 一致
  - **Type**: `rule`
  - **Covers**: AC-4
  - **Evidence**: Pending

- [ ] CP-R4: 电解机运行时每 tick 扣 10 EU；断电时 setHasPowerThisTick(false) 且 update 不推进
  - **Type**: `rule`
  - **Covers**: AC-3, AC-11
  - **Evidence**: Pending

- [ ] CP-R5: 存档升 v7，loadGame 接受 v2..v7，v≤6 不读最后 2 int
  - **Type**: `rule`
  - **Covers**: AC-6
  - **Evidence**: Pending

- [ ] CP-R6: 新机器 Z（电解机）、K（化合台）可放置、E 键打开 UI、字符不与现有冲突
  - **Type**: `rule`
  - **Covers**: AC-7, AC-10
  - **Evidence**: Pending

- [ ] CP-R7: BuildUI 列表顺序与 tryPlace case 序号一致（Gen/Wire/Crusher/Washer/Centrifuge/Sorter/Electrolyzer/ChemBench/Flower/Grass）
  - **Type**: `rule`
  - **Covers**: AC-7
  - **Evidence**: Pending

- [ ] CP-R8: TradeUI BUY tab 含 salt、Electrolyzer BP(300)、ChemBench BP(180)；元素不在 BUY
  - **Type**: `rule`
  - **Covers**: AC-5
  - **Evidence**: Pending

- [ ] CP-R9: 背包新增 ELEMENTS 与 COMPOUNDS 分类
  - **Type**: `rule`
  - **Covers**: AC-1, AC-8
  - **Evidence**: Pending

- [ ] CP-R10: MSBuild Debug x64 EXIT CODE = 0，单 UTF-8 BOM
  - **Type**: `rule`
  - **Covers**: AC-8, NFR-3, NFR-4
  - **Evidence**: Pending

- [ ] CP-U1: 化学系统经济平衡
  - **Type**: `rubric`
  - **Covers**: AC-9
  - **Scale**: 1-5
  - **Anchors**: 1 = 元素售价低于原料成本（亏本）；3 = 微利或与冶炼相当；5 = 电解/化合有明确高于直接卖原料的利润但不过度碾压
  - **Pass Threshold**: >= 4
  - **Evidence**: Pending

## Review History

### Review R1
- **Result**: `fail`
- **Evidence**: 独立代码审查；构建 EXIT=0、单 BOM；10 项检查中 9 项 PASS，CP-R4 FAIL（电解机未接入电网）
- **Actionable Findings**:
  - #1 (P0): tickPowerGrid 的 isConductive 与 poweredMachines 循环未包含 'Z'（同时遗漏已有的 'S'），电解机/筛选机永远不通电
  - #2 (P2): water 分类 "basic" 在背包无对应标签页，合成/初始水不可见
  - #3 (P2): copper_sulfate 化合配方亏本（投入 188c / 产出 180c）
- **Remediation Applied**:
  - #1: isConductive 加入 S/s/Z/z/K/k；poweredMachines 循环加入 'S'、'Z'
  - #2: 背包 cats 新增 `{ "basic", "BASIC / RAW", Color::BlueLight }` 标签页
  - #3: copper_sulfate 售价 180→220c
- **Re-verification**: 见 Review R2

### Review R2
- **Result**: `pass`
- **Evidence**:
  - #1 修复验证：L4676 isConductive 含 `S/s/Z/z/K/k`；L4697 poweredMachines 循环含 `'S'`、`'Z'`。电解机/筛选机现可被 BFS 发现并注册通电。
  - #2 修复验证：L3881 背包 cats 新增 `{ "basic", "BASIC / RAW", Color::BlueLight }`，water 现可见。
  - #3 修复验证：L538 copper_sulfate 售价 220c，毛利 220-188=32c > 0。
  - 全部 11 项检查点（CP-R1..CP-R10、CP-U1）通过；MSBuild Debug x64 EXIT=0；单 BOM。
- **Checkpoint Results**:
  - CP-R1..CP-R3, CP-R5..CP-R10 (`rule`): `pass`
  - CP-R4 (`rule`): `pass`（电网已接入 Z/S）
  - CP-U1 (`rubric`): `pass`；score 4；copper_sulfate 现已盈利
- **Findings**: 无 actionable 发现


