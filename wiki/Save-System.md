# Save System

存档系统详解。

---

## 📁 存档文件

| 文件 | 用途 | 写入时机 |
|------|------|----------|
| `chemical_world_slot1.sav` | 手动档 1 | 按 `P` 保存到 `activeSaveSlot=1` |
| `chemical_world_slot2.sav` | 手动档 2 | 按 `P` 保存到 `activeSaveSlot=2` |
| `chemical_world_slot3.sav` | 手动档 3 | 按 `P` 保存到 `activeSaveSlot=3` |
| `chemical_world_autosave.sav` | 自动档 | 退出游戏（按 `Q`）时自动写入 slot 0 |

> 路径 = 游戏可执行文件所在目录。

---

## 📝 v3 存档格式

文件头：`CHEMICAL_WORLD_SAVE 3`

### 完整结构

```
CHEMICAL_WORLD_SAVE 3
<player.name>
<level> <coins> <exp> <x> <y>
<currentArea int>
<inventory.size()>
<item.name> <qty> <category> <value>     ← 每行一个物品
<tutorials.size()>
<tut0_completed> <tut1_completed> ...    ← 0/1 序列
<width> <height>                          ← GameMap
<tile[y][x] for each row>                 ← 每行 width 个 tile
<gen_blueprint_unlocked 0/1> <wire_blueprint_unlocked 0/1>
<globalEU>
<machineMeta.size()>
<x> <y> <type 'F'/'L'/'G'> <remainingBurnEU> <loadedCoal> <active 0/1>   ← 每行一个机器
```

### 字段说明

| 字段 | 类型 | 说明 |
|------|------|------|
| `currentArea` | int | 0=Home, 1=Wasteland, 2=Cave |
| `category` | string | "basic" / "fuel" / "material" / "product" / "misc" |
| `tutorials.size()` | int | 5（教程步骤数） |
| `tile` | struct | `{ display char, name, interactText, passable bool, color int }` |
| `type` | char | `'F'` 高炉 / `'L'` 车床 / `'G'` 发电机 |
| `remainingBurnEU` | int | 发电机剩余 EU（其他机器为 0） |
| `loadedCoal` | int | 发电机已加载煤数（其他机器为 0） |
| `active` | bool | 发电机是否正在燃烧 |

---

## 🔄 v2 兼容

旧版 v2 存档可正常加载，格式：

```
CHEMICAL_WORLD_SAVE 2
... (同 v3 但缺少最后一段 v3 追加段)
```

### v2 加载逻辑

```cpp
if (version == 2) {
    // 1. 加载 player / map / inventory / tutorials
    // 2. 默认电力状态
    gen_blueprint_unlocked = false;
    wire_blueprint_unlocked = false;
    globalEU = 0;
    generators.clear();
    poweredMachines.clear();
    machineMeta.clear();
    // 3. 扫描加载后的地图，找 'F' / 'L' 大写字母作为锚点
    for each tile in map:
        if display == 'F': machineMeta.push_back({x, y, 'F', 0, 0, false});
        if display == 'L': machineMeta.push_back({x, y, 'L', 0, 0, false});
}
```

> ⚠️ v2 存档**不会**保留已购买的蓝图和 EU pool，加载后回到未解锁状态。建议加载后重新购买蓝图。

---

## 🎮 存档操作

### 保存（P 键）

```cpp
case 'P': case 'p':
    if (saveGame(activeSaveSlot))
        message = "Game saved to Manual Slot " + activeSaveSlot + ".";
    else
        message = "X Could not save the game.";
```

### 读取（L 键）

```cpp
case 'L': case 'l':
    if (loadGame(activeSaveSlot))
        message = "Game loaded from Manual Slot " + activeSaveSlot + ".";
    else
        message = "X No valid save in Manual Slot " + activeSaveSlot + ".";
```

### 启动时选择存档

游戏启动时会显示存档选择菜单（slot 1/2/3），读取失败则启动新游戏：
```cpp
if (!loadGame(selectedSlot)) newGame();
```

### 退出（Q 键）

```cpp
case 'Q': case 'q':
    saveGame(0);  // 自动保存到 autosave
    running = false;
```

---

## 🛠️ 存档相关代码位置

| 函数 | 位置 | 说明 |
|------|------|------|
| `savePath(slot)` | L1874 | 返回存档路径，slot 0 = autosave |
| `saveGame(slot)` | L1879 | 写入 v3 格式 |
| `loadGame(slot)` | L1912 | 读 v2/v3，v2 走兼容分支 |
| `newGame()` | L1993 | 重置所有状态，含电力与 machineMeta 默认值 |
| `selectSaveSlot()` | - | 启动时的存档选择 UI |

---

## ⚠️ 常见问题

**Q：手动档满了怎么办？**
A：游戏只有 3 个手动档位，加上 1 个 autosave。可以手动删除 `.sav` 文件释放位置，但游戏中**无法删除存档**。

**Q：v2 存档的蓝图会丢吗？**
A：会。v2 没有 `genBP wireBP` 字段，加载后默认 `false`。建议加载 v2 后立即用 `T` 重新购买蓝图。

**Q：autosave 会覆盖手动档吗？**
A：不会。autosave 写入 slot 0，手动档是 slot 1/2/3，互不干扰。

**Q：跨版本加载会损坏存档吗？**
A：v3 加载 v2 安全（走兼容分支）。但 v2 加载 v3 文件**会失败**（版本号不匹配），不会损坏 v3 文件本身。

**Q：背包里有特殊字符的物品名怎么办？**
A：物品名都是 ASCII（如 `steel_gear`），无空格，存档格式用空格分隔安全。

---

## 📊 存档示例

### v3 文件开头

```
CHEMICAL_WORLD_SAVE 3
Chemist
3 250 45 10 5
0
5
water 5 basic 3
coal 8 fuel 8
sand 3 material 2
steel 4 product 30
steel_gear 2 product 25
5
1 1 1 1 0
60 50
... (60×50 tile 数据)
1 1
6400
3
5 5 F 0 0 0
8 5 L 0 0 0
15 5 G 3200 1 1
```

### 解读

- 玩家 Chemist，3 级，250 coins，位置 (10, 5)
- 在家园（Area 0）
- 背包 5 种物品
- 教程 5 步，前 4 步完成
- 地图 60×50
- 两个蓝图都解锁
- EU pool = 6400
- 3 台机器：高炉 (5,5)、车床 (8,5)、发电机 (15,5)（剩余 3200 EU，1 块煤，正在燃烧）
