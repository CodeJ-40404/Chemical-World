# Proposal Standard / 提案标准

> **EN:** How to propose a new feature, machine, system or balance change for Chemical-World. Following this standard keeps every change reviewable, testable and backward-compatible.
> **中文：** 如何为 Chemical-World 提出新功能、新机器、新系统或平衡性改动。遵循本标准可保证每次变更可评审、可测试、向后兼容。

**Related / 相关：** [Join Us](Join-Us.md) · [Contributing Guide](../CONTRIBUTING.md) · [Machines](Machines.md) · [Save System](Save-System.md)

---

## 1. When a proposal is required / 何时需要提案

**EN:** A proposal is required for anything that changes gameplay data or systems. It is **not** required for typo fixes, comment/doc edits, or refactors with identical behavior.

**中文：** 任何改变玩法数据或系统的改动都需要提案。错别字修正、注释/文档编辑、行为完全不变的重构**不**需要提案。

| Needs a proposal / 需要提案 | No proposal needed / 无需提案 |
|------------------------------|-------------------------------|
| New machine, recipe, item, element, compound / 新机器、配方、物品、元素、化合物 | Typo / comment fixes / 错别字、注释修正 |
| New map, biome, entity, UI screen / 新地图、群系、实体、UI 界面 | Pure refactor (identical behavior) / 纯重构（行为不变） |
| Power/EU or price balance changes / 电力/EU 或价格平衡调整 | Build script / tooling fixes / 构建脚本、工具修复 |
| Save format change / 存档格式变更 | Wiki/README documentation / 文档更新 |
| New dependency / 新依赖 | Test-only changes / 仅测试改动 |

---

## 2. Process / 流程

**EN:**

1. **Search first** — check GitHub Issues, `RELEASES.md` roadmap, and `.trae/specs/` for duplicates or prior decisions.
2. **Open a tracking Issue** — label `proposal`, use the title format `[Proposal] <Name>`.
3. **Write the spec** — copy the template below into `.trae/specs/<kebab-case-name>/spec.md` (maintainers can do this for you after triage).
4. **Decision** — a maintainer marks the proposal `Accepted`, `Changes Requested`, `Deferred`, or `Rejected`, with a one-line rationale.
5. **Tasks & review** — accepted specs are broken into `tasks.md`; implementation must pass an independent review recorded in `review.md` before release.
6. **Docs & release** — update wiki + `RELEASES.md` in the same change.

**中文：**

1. **先搜索** —— 查看 GitHub Issues、`RELEASES.md` 路线图与 `.trae/specs/`，避免重复或与既有决策冲突。
2. **开追踪 Issue** —— 打上 `proposal` 标签，标题格式 `[Proposal] <名称>`。
3. **撰写规格** —— 复制下方模板到 `.trae/specs/<kebab-case-名称>/spec.md`（分流后维护者可代建）。
4. **决策** —— 维护者将提案标记为 `Accepted` / `Changes Requested` / `Deferred` / `Rejected`，并附一行理由。
5. **任务与审查** —— 接受的规格拆分为 `tasks.md`；实现须通过记录在 `review.md` 中的独立审查才能发布。
6. **文档与发布** —— 同一改动内更新 wiki 与 `RELEASES.md`。

```
DRAFT ──► triage ──► Accepted ──► tasks.md ──► implement ──► review ──► release
   │                 │                                                │
   │            Changes Rejected ◄────────── fail ◄───────────────────┘
   └─► Rejected / Deferred
```

---

## 3. Acceptance bar / 准入标准

**EN:** Every accepted proposal must satisfy all of the following.

**中文：** 每个被接受的提案必须满足以下全部条件。

| # | Requirement / 要求 | EN | 中文 |
|---|--------------------|----|------|
| R1 | **Bounded scope** | Clear Goals / Non-Goals; no silent scope creep | 明确 Goals / Non-Goals，不悄悄扩范围 |
| R2 | **Testable acceptance criteria** | Each AC is a `rule` (boolean) or `rubric` (1–5 with anchors + threshold) | 每条 AC 为 `rule`（布尔）或 `rubric`（1–5 分量表 + 阈值） |
| R3 | **Backward-compatible saves** | Bump save version; old saves (v2+) must load; state defaults given for missing fields | 提升存档版本；v2+ 旧档可加载；缺失字段给默认值 |
| R4 | **Economy sanity check** | New value chain must not strictly dominate or be strictly worse than an existing route | 新价值链不能严格碾压或严格劣于既有路线 |
| R5 | **Power integration** | New powered machines register in grid BFS (`isConductive` + `poweredMachines`) and use `powerDraw` | 新耗电机器须注册进电网 BFS 并使用 `powerDraw` |
| R6 | **Character collision audit** | New map chars must not collide with ore/decor/machine chars | 新地图字符不得与矿石/装饰/机器字符冲突 |
| R7 | **Build gate** | MSBuild Debug x64 `EXIT=0`; single UTF-8 BOM; no new warnings | MSBuild Debug x64 `EXIT=0`；单 BOM；无新增警告 |
| R8 | **No magic crafting** | Recipes follow real chemistry/industry logic wherever feasible (project vision) | 配方尽量遵循真实化学/工业逻辑（项目愿景） |
| R9 | **Bilingual docs** | User-facing wiki/RELEASES entries in English + 中文 | 面向玩家的 wiki/RELEASES 内容中英双语 |

---

## 4. Spec template / 规格模板

**EN / 中文：** Copy this into `.trae/specs/<name>/spec.md` / 复制到 `.trae/specs/<名称>/spec.md`。

```markdown
# <Feature Name>（<中文名>）- 产品需求文档 / PRD

## Overview
- **Summary / 摘要**：one paragraph / 一段话
- **Purpose / 目的**：what player problem this solves / 解决什么玩家问题
- **Target Users / 目标用户**：early / mid / late-game players / 前/中/后期玩家

## Goals
- ...

## Non-Goals
- Explicitly excluded scope / 明确排除的范围

## Background & Context
- Existing systems this touches / 涉及的既有系统
- Relevant item names, map chars, save fields / 相关物品名、地图字符、存档字段

## Functional Requirements
- **FR-1（<short name>）**：observable behavior, with exact numbers / 可观察行为，含确切数值
- **FR-2 ...**

## Non-Functional Requirements
- **NFR-1**：single-file constraint (Chemical-World.cpp only) / 单文件约束
- **NFR-2**：FTXUI components mounted in Container AND rendered (no dangling refs) / 防悬空
- **NFR-3**：single BOM / 单 BOM
- **NFR-4**：MSBuild EXIT=0

## Constraints
- **Technical**：FTXUI 6.1.9 caveats (no Element Maybe, no DarkYellow, no Event::Space)
- **Business**：cost/positioning / 成本与定位
- **Dependencies**：systems relied upon / 依赖的系统

## Assumptions
- ...

## Acceptance Criteria
### AC-1：<name>
- **Type**: `rule` | `rubric`
- **Given**：...
- **When**：...
- **Then**：...
- **Pass Condition**：...
- **Evidence**：manual test / code grep / build output / 手测/代码检查/构建输出
# rubric only: **Scale** 1-5, **Anchors**, **Pass Threshold**

## Open Questions
- [ ] unresolved decisions / 未决问题
```

### AC writing rules / AC 编写规则

**EN:**
- Be executable: another person must be able to decide pass/fail without asking you.
- Use exact numbers (EU, ms, coins, item counts), never "fast" or "reasonable".
- Name exact identifiers (`bauxite_dust`, `powerDraw(10, pos)`, save header `CHEMICAL_WORLD_SAVE 7`).
- At least one AC must cover **old-save loading** whenever the save version changes.

**中文：**
- 可执行：他人无需询问你即可判定通过/失败。
- 用确切数值（EU、毫秒、金币、数量），不写"较快""合理"。
- 写准确切标识符（`bauxite_dust`、`powerDraw(10, pos)`、存档头 `CHEMICAL_WORLD_SAVE 7`）。
- 只要提升存档版本，至少一条 AC 必须覆盖**旧档加载**。

---

## 5. Save-version checklist / 存档版本检查单

**EN / 中文：** Required whenever a proposal persists new state / 凡需持久化新状态时必查。

- [ ] Increment header, e.g. `CHEMICAL_WORLD_SAVE 7 → 8` / 递增文件头版本号
- [ ] Write new field only in the new version branch / 新字段只在新版本分支写出
- [ ] Accept every prior version (`v2…current`); missing fields get explicit defaults / 接受所有旧版本，缺失字段显式默认
- [ ] `newGame()` resets the new state / 新游戏重置新状态
- [ ] Machine instances stay unserialized — rebuild via `machineMeta` + rehydrate / 机器实例不序列化，走 meta 重建
- [ ] Manual round-trip tested: old save → load → save → load / 手测旧档→加载→保存→再加载

---

## 6. New machine checklist / 新机器检查单

**EN / 中文：** Based on the standard integration steps in [CONTRIBUTING](../CONTRIBUTING.md).

- [ ] Machine class: `AnimState` + `Recipe` + `canLoad/craft` + `update()` / 机器类
- [ ] Character pair chosen & collision-audited (`Z/z`, `K/k`…) / 字符对与冲突审计
- [ ] `placeXxx()` cost + 2×2 stamping + `machineMeta` entry / 放置函数与扣费
- [ ] Build menu entry **and** matching `tryPlace` case index / 建造菜单项与 tryPlace 序号一致
- [ ] Blueprint in TradeUI BUY table (one-time) + reset/load flags / 蓝图与存档标记
- [ ] If powered: `isConductive`, `poweredMachines` loop, `powerDraw` in `globalTick` / 耗电机器的电网注册
- [ ] E-key `findNearbyMachine('<char>')` dispatch / E 键分发
- [ ] Full-screen FTXUI UI (components in Container + Renderer; ticker only if it ticks) / 全屏 UI
- [ ] Backpack categories for outputs via `chemOutputCategory`-style helper / 产出分类
- [ ] Home legend + build preview colors / 家园图例与预览配色
- [ ] itemPrice for every sellable output / 每个可售产物的 itemPrice

---

## 7. Review standard / 审查标准

**EN:** Before release, an independent reviewer (not the implementer) fills `review.md`:

**中文：** 发布前由独立审查者（非实现者本人）填写 `review.md`：

- One checkpoint per AC, plus integration checkpoints (grid, save, build/BOM, character audit) / 每条 AC 一个检查点，另加集成检查点
- Result per checkpoint: `pass` / `fail` / `blocked`, with file:line evidence / 逐项给结论与 file:line 证据
- Findings classified **P0** (blocks release) / **P1** (must fix soon) / **P2** (polish) / 问题分级 P0/P1/P2
- Any `fail` becomes an Issue entry in `tasks.md`, is fixed, then re-reviewed (`R2`) until `pass` / 失败项转 tasks.md Issue，修复后 R2 复查至 pass

---

## 8. Example / 参考实例

**EN / 中文：** The reference proposal that defined this standard is the Electrolysis Update:
定义本标准的参考实例为电解更新：

- Spec / 规格：`.trae/specs/electrolysis-update/spec.md`
- Tasks / 任务：`tasks.md` (12 tasks + 3 review Issues)
- Review / 审查：`review.md` (R1 fail → fixes → R2 pass)
- Release / 发布：[`RELEASES.md` v0.5.0](../RELEASES.md)
