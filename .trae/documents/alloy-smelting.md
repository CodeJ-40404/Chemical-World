# 合金冶炼 (Alloy Smelting) — 实施计划

## Context（为什么做这个）
矿物处理链已完整闭环并构建通过（矿石→破碎→纯净→矿粉→锭，GT:NH ×2）。目前高炉只能把单一矿粉/矿石烧成单质锭，没有**合金**这一环。合金冶炼直接消费处理链产物（铜/锡/金/银的锭或矿粉），合成更高价值的合金锭（青铜、琥珀金），闭合"材料深加工→高价值产物"的游戏循环，并与既有的"ingot 路径 + dust 路径 ×2"配方双轨惯例保持一致。

目标：在现有 `BlastFurnace` 上扩展**可选第二输入**，新增 4 条合金配方（2 种合金 × ingot/dust 双路径），并在 UI 与售价表中同步支持。**不改存档版本**（配方是静态的，合金锭走通用 inventory 序列化）。

## 设计要点
- **合金集**（基于现有金属：copper/tin/gold/silver；无 zinc 故不做 brass）：
  - 青铜 Bronze = copper + tin
  - 琥珀金 Electrum = gold_ingot + silver_ingot
- **双路径**（沿用高炉既有"ore/ingot 路径 + dust 路径 ×2"惯例）：
  - ingot 路径：2 copper + 1 tin → 3 bronze_ingot；1 gold_ingot + 1 silver_ingot → 2 electrum_ingot
  - dust 路径（×2 奖励）：2 malachite_dust + 1 cassiterite_dust → 6 bronze_ingot；1 gold_dust + 1 silver_dust → 4 electrum_ingot
  - 注：无通用 "copper_dust" 主产物（copper_dust 仅洗矿槽稀有副产），故青铜 dust 路径用 `malachite_dust` 作铜源。
- **配方结构扩展**：在 `Recipe` 末尾追加 `string oreName2; int oreRequired2 = 0;`（C++20 aggregate + NSDMI）。现有 16 条 7 字段配方不动（oreName2 值初始化为 ""，oreRequired2=0）；新合金配方用 9 字段。
- **无蓝图门槛**：青铜/琥珀金需要完整处理链（crusher/washer/centrifuge 已各自蓝图门控），合金本身不再加蓝图，与现有高炉配方（全部默认可用）一致。

## 关键文件
仅一个文件：`d:\c++\repos\Chemical-World\Chemical-World\Chemical-World.cpp`

## 实施步骤

### S1. Recipe 结构扩展（~L210-L218）
在 `struct Recipe` 末尾 `int durationMs;` 之后追加：
```cpp
string oreName2;            // 合金第二输入（默认空 = 单输入配方）
int oreRequired2 = 0;
```

### S2. 新增 4 条合金配方（recipes 向量末尾，~L244 `"Silver From Dust"` 之后）
```cpp
// 合金冶炼：ingot 路径（基础产出）
{"Bronze Alloy",   "copper",     "bronze_ingot",  2, 1, 3, 30000, "tin", 1},
{"Electrum Alloy", "gold_ingot", "electrum_ingot",1, 1, 2, 30000, "silver_ingot", 1},
// 合金冶炼：dust 路径（×2 奖励，消费处理链产物）
{"Bronze From Dust",  "malachite_dust", "bronze_ingot",  2, 1, 6, 30000, "cassiterite_dust", 1},
{"Electrum From Dust","gold_dust",      "electrum_ingot",1, 1, 4, 30000, "silver_dust", 1}
```

### S3. canLoad / loadMaterials 支持第二输入（~L337-L381）
- `canLoad(player)`：coal 检查后追加 `&& (r.oreName2.empty() || player.hasItem(r.oreName2, r.oreRequired2))`
- `loadMaterials(player)`：`removeItem("coal",…)` 后追加 `if (!r.oreName2.empty()) player.removeItem(r.oreName2, r.oreRequired2);`
- `addLog` 装载消息：`!r.oreName2.empty()` 时追加 ` + r.oreRequired2 + " " + r.oreName2`

### S4. 配方信息显示（getRecipeInfo / getRecipeList，~L321-L333）
两者在显示串里，当 `!r.oreName2.empty()` 时追加 `" + " + r.oreName2 + " x" + to_string(r.oreRequired2)`

### S5. FurnaceUI 配方名与详情（setupUI + mainContainer）
- setupUI `recipeNames`（~L1441）：追加 `+ (!r.oreName2.empty() ? "+" + r.oreName2 + "x" + to_string(r.oreRequired2) : "")`
- mainContainer `recipeInfo`（~L1548-L1550）：括号内追加第二输入显示
- mainContainer Bag 区（~L1551 + ~L1584）：
  - 新增 `int ore2Cnt = r.oreName2.empty() ? 0 : player.getItemCount(r.oreName2);`
  - Bag 行里当 `!r.oreName2.empty()` 时插入 `text("   " + r.oreName2 + " x" + to_string(ore2Cnt)) | color(Color::RedLight)`，放在主料与 coal 之间
- LOAD 失败提示（~L1469）cosmetic 改为 "Need recipe inputs + coal"

### S6. 售价表（itemPrice，~L125）
在 `silver_ingot` 条目后追加：`{"bronze_ingot", 55}, {"electrum_ingot", 90},`
（TradeUI `rebuildSell()` 自动列出全部库存物品，有 itemPrice 即可售；不会进 BUY tab，符合"产物不可购买"。）

### S7.（可选清理）标注遗留死代码
`findRecipe(3 参)`、`canLoad(player,name,amt,fuel)`、`loadMaterials(player,name,amt,fuel)`、`canStart/start` 均无外部调用（仅自引用），且**不支持合金第二输入**。加注释 `// LEGACY: not alloy-aware`，避免日后被误用导致漏扣第二原料。不删除以缩小 diff。

## 不需要改动（已确认）
- `update()` 产出逻辑：`player.addItem(r.result, s.resultCount, "product", 20)` 对合金天然有效（r.result="bronze_ingot"）。
- 存档：配方是静态 vector，不序列化；`bronze_ingot`/`electrum_ingot` 走通用 inventory（name/qty/category/value）。**保持 v4，不升版本**。
- `selectedRecipe` 是运行时成员、不存盘；新配方加在末尾（索引 16-19），不动 0-15，存档兼容。
- newGame/loadGame：`furnace = BlastFurnace{}` 重建配方表，自动包含新合金配方。

## 验证（构建 + 手动）
1. MSBuild 构建验证（EXIT=0）：
   ```
   D:\c++\for-vs\MSBuild\Current\Bin\MSBuild.exe Chemical-World.vcxproj /p:Configuration=Debug /p:Platform=x64 /m /v:minimal
   ```
   预期：无 error；仅既有 C4566 Unicode 警告与 LNK4099 PDB 警告。
2. 手动 E2E（运行 .exe）：
   - Home 起步 → 挖 malachite + cassiterite（或 gold/silver 矿）→ 破碎→洗净→离心得到 malachite_dust + cassiterite_dust（或 gold_dust + silver_dust）。
   - 高炉 F：配方列表应出现 4 条新合金配方，且 ingot/dust 路径都显示第二输入（如 `malachite_dust (2+c1->bronze_ingot)+cassiterite_dustx1`）。
   - 选中 "Bronze From Dust" → Bag 区显示 malachite_dust / cassiterite_dust / coal 三项库存 → LOAD 成功、30s 后 COLLECT 得到 6 bronze_ingot。
   - T 交易界面 SELL：bronze_ingot @ 55c、electrum_ingot @ 90c 可售。
3. 回归：旧配方（Steel/Copper/Gold…）仍可正常 LOAD/COLLECT；旧存档（v2/v3/v4）可加载。
