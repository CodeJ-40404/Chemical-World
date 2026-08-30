# 安全政策 / Security Policy

[English](#english) | [中文](#中文)

---

## 中文版

### 📋 概述

Chemical-World 是一个终端游戏项目，主要面向开发者和技术爱好者。虽然本项目不处理敏感数据或网络通信，但我们仍然重视安全性，并欢迎社区报告任何潜在问题。

---

### 🔒 支持的安全版本

| 版本 | 支持状态 | 安全更新 |
|------|---------|----------|
| main 分支 | ✅ 支持中 | 即时修复 |
| Save v5.x | ✅ 支持中 | 即时修复 |
| Save v4.x | ⚠️ 有限支持 | 仅严重漏洞 |
| Save v3.x 及更早 | ❌ 不再支持 | 不提供修复 |

> **建议**：始终使用最新版本或 `main` 分支。旧版本可能存在已知问题，且不受安全更新覆盖。

---

### 🐛 报告安全漏洞

如果你发现安全漏洞，请**不要**在公开的 GitHub Issues 中披露，以防被恶意利用。

#### 报告方式

1. **首选**：发送邮件至项目维护者（通过 GitHub 个人资料中的公开邮箱）
2. **备选**：在 GitHub 上创建 **Private Security Advisory**
   - 访问：`https://github.com/CodeJ-40404/Chemical-World/security/advisories/new`
   - 填写漏洞描述、影响范围和复现步骤

#### 报告内容应包含

- **漏洞类型**（如：存档注入、整数溢出、UI 崩溃等）
- **影响版本**（你测试的版本/分支）
- **复现步骤**（详细到可操作）
- **潜在影响**（可能造成的后果）
- **建议修复方案**（如你有想法）

#### 响应时间线

| 阶段 | 时间 |
|------|------|
| 确认收悉 | 48 小时内 |
| 初步评估 | 7 天内 |
| 修复发布（严重） | 14 天内 |
| 修复发布（一般） | 30 天内 |

---

### ⚠️ 已知安全边界

#### 存档系统

- 存档文件是**未加密的二进制格式**，可被手动编辑
- 恶意修改存档可能导致游戏崩溃或进度丢失
- **建议**：仅信任自己生成的存档，不要共享 `.sav` 文件

#### 第三方依赖

| 依赖 | 版本 | 已知漏洞 |
|------|------|----------|
| FTXUI | 6.1.9 | 无已知 CVE |
| nlohmann/json | 3.11.2 | 无已知 CVE |

> 依赖更新时会同步审查安全公告。

#### 平台限制

- 当前仅支持 Windows 10/11 
- 使用 `windows.h` API，不涉及网络或系统级权限提升
- 不读取/写入系统目录之外的文件（仅项目目录）

#### 输入安全

- 键盘输入通过 `_getch()` 捕获，无注入风险
- 存档解析使用流式读取，已处理异常情况（`try-catch` 包裹）

---

### 🔧 开发中的安全实践

#### 代码层面

| 实践 | 说明 |
|------|------|
| 异常处理 | 文件 I/O、JSON 解析均用 `try-catch` 捕获 |
| 边界检查 | 数组/向量访问前检查 `size()` 边界 |
| 输入验证 | 存档版本号、坐标、数量均有合法性校验 |
| 内存管理 | 使用 `std::vector` / `std::string`，避免裸指针 |

#### 已处理的风险

1. **存档损坏** → `loadGame()` 失败时回退到 `newGame()`
2. **配方越界** → 访问前检查 `selectedRecipe < recipes.size()`
3. **坐标越界** → `GameMap::getTile()` 返回静态 `wall` 占位
4. **JSON 解析错误** → `loadConfig()` 捕获异常并回退到内置任务列表
5. **空指针/悬空引用** → FTXUI 组件使用 `Maybe` 门控和成员变量存储（见贡献指南）

---

### 📢 安全披露政策

- 我们会在修复漏洞后 **7 天内** 发布安全公告
- 公告将包含漏洞类型、影响版本和修复版本
- 安全公告将发布于 GitHub Security Advisories 和项目 Releases

#### 致谢

我们会公开致谢报告安全漏洞的贡献者（除非你要求匿名）。

---

### 📬 联系方式

- **安全问题专用邮箱**：通过 GitHub 个人资料联系 [@CodeJ-40404](https://github.com/CodeJ-40404)
- **紧急联系**：在 GitHub Issues 中标记 `[SECURITY]` 前缀（仅限已确认的漏洞，优先使用邮件）

---

**感谢你帮助保障 Chemical-World 的安全！** 🛡️

---

---

## English

### 📋 Overview

Chemical-World is a terminal-based game project primarily aimed at developers and tech enthusiasts. While this project does not handle sensitive data or network communications, we still take security seriously and welcome the community to report any potential issues.

---

### 🔒 Supported Security Versions

| Version | Support Status | Security Updates |
|---------|----------------|------------------|
| `main` branch | ✅ Supported | Immediate fixes |
| Save v5.x | ✅ Supported | Immediate fixes |
| Save v4.x | ⚠️ Limited Support | Critical only |
| Save v3.x and earlier | ❌ Unsupported | No fixes provided |

> **Recommendation**: Always use the latest version or `main` branch. Older versions may have known issues and are not covered by security updates.

---

### 🐛 Reporting Security Vulnerabilities

If you discover a security vulnerability, please **do not** disclose it in public GitHub Issues to prevent malicious exploitation.

#### How to Report

1. **Preferred**: Send an email to the project maintainer (via the public email on GitHub profile)
2. **Alternative**: Create a **Private Security Advisory** on GitHub
   - Visit: `https://github.com/CodeJ-40404/Chemical-World/security/advisories/new`
   - Fill in vulnerability description, impact scope, and reproduction steps

#### What to Include in Your Report

- **Vulnerability Type** (e.g., save injection, integer overflow, UI crash, etc.)
- **Affected Version** (version/branch you tested)
- **Reproduction Steps** (detailed and actionable)
- **Potential Impact** (possible consequences)
- **Suggested Fix** (if you have an idea)

#### Response Timeline

| Phase | Timeframe |
|-------|-----------|
| Acknowledgment | Within 48 hours |
| Initial Assessment | Within 7 days |
| Fix Release (Critical) | Within 14 days |
| Fix Release (Normal) | Within 30 days |

---

### ⚠️ Known Security Boundaries

#### Save System

- Save files are **unencrypted binary format** and can be manually edited
- Malicious modification may cause game crashes or progress loss
- **Recommendation**: Only trust saves you generated yourself; do not share `.sav` files

#### Third-party Dependencies

| Dependency | Version | Known Vulnerabilities |
|------------|---------|----------------------|
| FTXUI | 6.1.9 | No known CVEs |
| nlohmann/json | 3.11.2 | No known CVEs |

> Security bulletins are reviewed when dependencies are updated.

#### Platform Limitations

- Currently only supports Windows 10/11
- Uses `windows.h` API, no network or system-level privilege escalation
- Does not read/write files outside the project directory

#### Input Security

- Keyboard input captured via `_getch()`, no injection risk
- Save parsing uses streaming reads with exception handling (`try-catch` wrapped)

---

### 🔧 Security Practices in Development

#### Code-Level Practices

| Practice | Description |
|----------|-------------|
| Exception Handling | File I/O and JSON parsing wrapped with `try-catch` |
| Boundary Checks | Check `size()` boundaries before array/vector access |
| Input Validation | Save version, coordinates, quantities all validated |
| Memory Management | Use `std::vector` / `std::string`, avoid raw pointers |

#### Handled Risks

1. **Save corruption** → `loadGame()` falls back to `newGame()` on failure
2. **Recipe out-of-bounds** → Check `selectedRecipe < recipes.size()` before access
3. **Coordinate out-of-bounds** → `GameMap::getTile()` returns static `wall` placeholder
4. **JSON parse errors** → `loadConfig()` catches exceptions and falls back to built-in quests
5. **Null/dangling references** → FTXUI components use `Maybe` guards and member variables (see Contribution Guide)

---

### 📢 Security Disclosure Policy

- We will publish a security advisory **within 7 days** after fixing a vulnerability
- Advisories will include vulnerability type, affected versions, and fixed versions
- Security advisories will be published on GitHub Security Advisories and project Releases

#### Acknowledgments

We will publicly acknowledge contributors who report security vulnerabilities (unless you request anonymity).

---

### 📬 Contact

- **Security Issues Dedicated Email**: Contact [@CodeJ-40404](https://github.com/CodeJ-40404) via GitHub profile
- **Urgent Contact**: Tag `[SECURITY]` prefix in GitHub Issues (only for confirmed vulnerabilities; email is preferred)

---

**Thank you for helping keep Chemical-World secure!** 🛡️

---

*Last updated: 2026-08-30*
