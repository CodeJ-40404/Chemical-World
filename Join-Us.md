# Join Us / 加入我们

> **EN:** Chemical-World is an open-source, single-file C++20 terminal game built by volunteers. You do not need to be a C++ expert to help — balancing, documentation, translation and testing all move the project forward.
> **中文：** Chemical-World 是一个由志愿者构建的开源单文件 C++20 终端游戏。你不必是 C++ 专家也能参与——数值平衡、文档、翻译与测试都能推动项目前进。

**Related / 相关：** [Proposal Standard](Proposal-Standard.md) · [Contributing Guide](../CONTRIBUTING.md) · [Code of Conduct](../CODE_OF_CONDUCT.md) · [Security Policy](../SECURITY.md)

---

## What we are building / 我们在做什么

**EN:** A hardcore sandbox fusing **industry → chemistry → space**, inspired by GT:New Horizons' power networks and machine chains. Our north star after the framework is complete: **as little "magic crafting" as possible** — recipes should follow real chemistry and industry.

**中文：** 一个融合**工业 → 化学 → 宇航**的硬核沙盒，灵感来自 GT:New Horizons 的电网与机器链。框架完工后的北极星：**尽可能没有魔法合成**——配方遵循真实化学与工业逻辑。

---

## Ways to contribute / 参与方式

| Role / 角色 | What it involves / 工作内容 | Skills / 技能 |
|-------------|------------------------------|---------------|
| **C++ Gameplay Dev / C++ 玩法开发** | Machines, recipes, map gen, FTXUI panels; single-file `Chemical-World.cpp` | C++20 basics, patience with legacy code / C++20 基础 |
| **Chemistry Designer / 化学设计** | Realistic reactions, stoichiometry in integer shares, pricing/EU balance | High-school+ chemistry, spreadsheets / 化学常识、表格 |
| **Economy Balancer / 数值平衡** | Prices, EU costs, XP, progression curves; write proposal ACs / 价格、EU、经验、成长曲线 | Spreadsheets, clear reasoning / 数据推演 |
| **Wiki & Docs / 文档** | Keep the bilingual wiki accurate; gameplay guides / 维护双语 wiki 与攻略 | Markdown, clear writing / Markdown、清晰表达 |
| **Translator EN ⇄ 中文 / 翻译** | Bilingual wiki, RELEASES, in-game text review (in-game text stays ASCII English) | 中英双语 |
| **ASCII / UI Artist / 界面美术** | FTXUI layouts, ASCII animations, color mapping / FTXUI 布局、ASCII 动画 | Taste for terminal UI / 终端审美 |
| **Tester / QA / 测试** | Playtest PRs, verify ACs, report repro steps / 试玩 PR、验证验收标准 | Thoroughness, good bug reports / 细致 |
| **Packaging / Porting / 打包移植** | Linux/macOS port (replace Windows API), CMake, CI / 跨平台移植 | CMake, POSIX / 平台经验 |

---

## Good first issues / 新手友好任务

**EN:** Sized to land in one evening. Search the `good first issue` label on GitHub; examples:

**中文：** 一个晚上即可完成的规模。在 GitHub 搜索 `good first issue` 标签，例如：

- Add a new electrolysis recipe **following the [Proposal Standard](Proposal-Standard.md)** (must be a real reaction) / 按提案标准新增一条电解配方（须为真实反应）
- Give `sulfur` / `carbon` / `nitrogen` / `magnesium` / `silicon` a real production source (sulfide ore? air separation?) / 为预留元素设计真实来源（硫化矿？空气分离？）
- Fix/enhance a wiki page bilingually / 双语修订 wiki 页面
- Add a new quest in `quests.json` / 在 `quests.json` 中新增任务
- Verify v2–v7 save loading and document edge cases / 验证 v2–v7 存档加载并记录边界情况
- Replace a Windows-only call to advance the Linux/macOS port / 替换 Windows 专属调用以推进移植

---

## Workflow in 5 steps / 五步工作流

**EN:**

1. Comment on (or open) an Issue and say what you plan to do — wait for a maintainer nod.
2. Fork → branch `feature/<short-name>` (or `fix/`, `docs/`).
3. Follow the [Contributing Guide](../CONTRIBUTING.md): MSBuild `EXIT=0`, no dangling FTXUI refs, English+ASCII in-game text.
4. Gameplay changes need a spec per the [Proposal Standard](Proposal-Standard.md) and a self-tested AC checklist.
5. Open a PR: describe **what**, **why**, **how you tested it**, and attach screenshots/text captures for UI changes.

**中文：**

1. 在 Issue 下留言说明你想做什么——等维护者确认。
2. Fork → 建分支 `feature/<短名>`（或 `fix/`、`docs/`）。
3. 遵循[贡献指南](../CONTRIBUTING.md)：MSBuild `EXIT=0`、FTXUI 引用不悬空、游戏内文本英文+ASCII。
4. 玩法改动须按[提案标准](Proposal-Standard.md)写规格，并自测验收清单。
5. 发起 PR：说明**改了什么**、**为什么**、**如何测试**；UI 改动附截图/文本录制。

### PR checklist / PR 检查单

- [ ] Builds with MSBuild Debug x64, `EXIT CODE = 0` / 构建零错误
- [ ] Single UTF-8 BOM; no new warnings / 单 BOM、无新增警告
- [ ] Save version bumped + old saves load (if persisting state) / 必要时升存档版本且旧档可加载
- [ ] New powered machine registered in grid BFS / 新耗电机器注册进电网
- [ ] New map chars collision-audited / 新字符冲突审计
- [ ] Wiki + RELEASES updated, bilingual / wiki 与 RELEASES 双语更新
- [ ] In-game strings are English + basic ASCII / 游戏内字符串英文+ASCII

---

## Recognition / 贡献认可

**EN:**
- All contributors are listed in the release notes of the version they shipped in.
- Notable recurring contributors are invited to become maintainers (merge rights, roadmap voice).
- This is a no-pressure project: review turnaround is "when people have time"; life always comes first.

**中文：**
- 所有贡献者会列入其贡献所在版本的发布说明。
- 持续高质量贡献者会被邀请成为维护者（合并权限、路线图话语权）。
- 项目零压力：评审"有空就做"，现实生活永远优先。

---

## Contact / 联系方式

| Channel / 渠道 | Purpose / 用途 | Link / 链接 |
|----------------|----------------|-------------|
| GitHub Issues | Bugs, proposals, tasks / 缺陷、提案、任务 | [issues](https://github.com/CodeJ-40404/Chemical-World/issues) |
| GitHub Discussions | Ideas & questions / 想法与提问 | [discussions](https://github.com/CodeJ-40404/Chemical-World/discussions) |
| Pull Requests | Code & docs / 代码与文档 | [pulls](https://github.com/CodeJ-40404/Chemical-World/pulls) |
| Maintainer / 维护者 | — | [@CodeJ-40404](https://github.com/CodeJ-40404) |
| Discord | Not yet — open an Issue "community: discord" if you want to run it / 暂未建立，愿意运营可提 Issue | — |

> **EN / 中文：** By participating you agree to the [Code of Conduct](../CODE_OF_CONDUCT.md). Be kind, be concrete, respect contributors' time.
> 参与即表示你同意[行为准则](../CODE_OF_CONDUCT.md)。友善、具体、尊重他人的时间。

---

*Pull request #1 is waiting. / 第一个 PR 在等你。* 🧪⚗️⚡
