// Chemical-world.cpp
// 使用 FTXUI 6.1.9 实现图形界面
// 特别警告：目前 !!!游戏内容!!! 请使用全英文+基础ascii以避免乱码，注释可以使用中文

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <stdio.h>
#include <map>
#include <set>
#include <ctime>
#include <random>
#include <thread>
#include <chrono>
#include <fstream>

using namespace ftxui;
using namespace std;

// ======================== 颜色定义 ========================
#define COLOR_BLACK 0
#define COLOR_DARK_BLUE 1
#define COLOR_DARK_GREEN 2
#define COLOR_DARK_CYAN 3
#define COLOR_DARK_RED 4
#define COLOR_DARK_PURPLE 5
#define COLOR_DARK_YELLOW 6
#define COLOR_RESET 7
#define COLOR_GREY 8
#define COLOR_BLUE 9
#define COLOR_GREEN 10
#define COLOR_CYAN 11
#define COLOR_RED 12
#define COLOR_PURPLE 13
#define COLOR_YELLOW 14
#define COLOR_LIGHT_WHITE 15

inline void setcolor(int color) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, color);
#endif
}

inline void cls() { system("cls"); }

// ======================== 数据结构 ========================
struct InventoryItem {
    string name;
    int quantity;
    string category;
    int value;
};

struct PlayerData {
    string name;
    int level = 1;
    int coins = 100;
    int exp = 0;
    int x = 5;
    int y = 5;
    vector<InventoryItem> inventory;

    void addItem(const string& name, int qty, const string& category = "misc", int value = 0) {
        for (auto& item : inventory) {
            if (item.name == name) {
                item.quantity += qty;
                return;
            }
        }
        inventory.push_back({ name, qty, category, value });
    }

    bool hasItem(const string& name, int qty = 1) {
        for (auto& item : inventory) {
            if (item.name == name && item.quantity >= qty) return true;
        }
        return false;
    }

    bool removeItem(const string& name, int qty = 1) {
        for (auto& item : inventory) {
            if (item.name == name) {
                if (item.quantity >= qty) {
                    item.quantity -= qty;
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    int getItemCount(const string& name) {
        for (auto& item : inventory) {
            if (item.name == name) return item.quantity;
        }
        return 0;
    }
};

// 物品售价表（文件级自由函数，供 TradeUI 等使用）
int itemPrice(const string& name) {
    static map<string, int> prices = {
        {"water", 3}, {"hematite", 5}, {"magnetite", 5}, {"bauxite", 4},
        {"cassiterite", 5}, {"malachite", 6}, {"chalcopyrite", 6},
        {"gold_ore", 20}, {"silver_ore", 12}, {"coal", 8},
        {"steel", 30}, {"glass", 15}, {"sand", 2},
        {"iron_ingot", 12}, {"alloy", 45},
        // 车床产品（steel 前缀）
        {"steel_gear", 25}, {"steel_rod", 20}, {"steel_plate", 18},
        {"steel_spring", 30}, {"steel_bolt", 12}, {"steel_wire", 35},
        // 金属锭（土高炉产物，之前漏了导致无法出售）
        {"aluminum", 15}, {"tin", 18}, {"copper", 20},
        {"gold_ingot", 60}, {"silver_ingot", 35},
        // 矿物处理链主产物（24 种）
        // 破碎矿石（crushed_*，破碎机产物）
        {"crushed_hematite", 8}, {"crushed_magnetite", 8}, {"crushed_bauxite", 8},
        {"crushed_cassiterite", 8}, {"crushed_malachite", 8}, {"crushed_chalcopyrite", 8},
        {"crushed_gold", 8}, {"crushed_silver", 8},
        // 纯净矿石（purified_*，洗矿槽产物）
        {"purified_hematite", 12}, {"purified_magnetite", 12}, {"purified_bauxite", 12},
        {"purified_cassiterite", 12}, {"purified_malachite", 12}, {"purified_chalcopyrite", 12},
        {"purified_gold", 12}, {"purified_silver", 12},
        // 矿粉（*_dust，离心机产物）
        {"hematite_dust", 18}, {"magnetite_dust", 18}, {"bauxite_dust", 18},
        {"cassiterite_dust", 18}, {"malachite_dust", 18}, {"chalcopyrite_dust", 18},
        {"gold_dust", 18}, {"silver_dust", 18},
        // 副产物
        {"gravel", 1}, {"copper_dust", 20}, {"rare_dust", 30}
    };
    auto it = prices.find(name);
    return it != prices.end() ? it->second : 0;
}

class ChemicalWorldGame;   // 前向声明，供 LatheUI / TradeUI 引用

// 机器元信息：存档里序列化每台机器的左上坐标/类型/燃烧状态
struct MachineMeta {
    int x, y;
    char type;  // 'F' / 'L' / 'G'
    int remainingBurnEU = 0;
    int loadedCoal = 0;
    bool active = false;
};

// 火力发电机
class PowerGenerator {
public:
    int burnEU = 0;
    int loadedCoal = 0;
    int frameIndex = 0;
    bool active = false;

    bool feedCoal(PlayerData& p) {
        if (!p.hasItem("coal", 1)) return false;
        p.removeItem("coal", 1);
        burnEU += 6400;
        loadedCoal += 1;
        active = true;
        return true;
    }

    // 返回 <注入EU, 是否仍在燃烧>
    pair<int, bool> updateTick() {
        if (burnEU <= 0) {
            active = false;
            return { 0, false };
        }
        int inject = min(8, burnEU);
        burnEU -= inject;
        frameIndex++;
        active = true;
        if (burnEU <= 0) {
            return { inject, false };
        }
        return { inject, true };
    }

    int getBurnEU() const { return burnEU; }
    bool isActive() const { return active; }
};

// ======================== 高炉系统 ========================
// 土高炉：4 槽并行，不耗电，不需要温度/吹空气。每槽 30s（300 × 100ms ticks）。
// 动画 24 帧：0-5 STOKING(投煤+鼓风), 6-17 HEATING(火焰), 18-23 POURING(出铁)
class BlastFurnace {
public:
    enum SlotPhase { IDLE, STOKING, HEATING, POURING, DONE };
    struct Slot {
        SlotPhase phase = IDLE;
        int       recipeIndex = -1;
        int       progressMs = 0;
        int       totalMs = 30000;
        int       loadedRecipe = -1;
        int       oreCount = 0;
        int       fuelCount = 0;
        int       resultCount = 0;
    };
    struct Recipe {
        string name;
        string oreName;
        string result;
        int oreRequired;
        int fuelRequired;
        int resultAmount;
        int durationMs;
    };

private:
    vector<string> logs;
    vector<Slot> slots;
    int focusedSlot = 0;
    int selectedRecipe = 0;

    vector<Recipe> recipes = {
        // 旧路径：直接烧矿石（1 ingot）
        {"Steel Making",  "hematite",    "steel",       2, 1, 1, 30000},
        {"Steel Making",  "magnetite",   "steel",       2, 1, 1, 30000},
        {"Aluminum Smelt","bauxite",     "aluminum",    2, 1, 1, 30000},
        {"Tin Smelting",  "cassiterite", "tin",         2, 1, 1, 30000},
        {"Copper Smelt",  "malachite",   "copper",      2, 1, 1, 30000},
        {"Copper Smelt",  "chalcopyrite","copper",      2, 1, 1, 30000},
        {"Gold Smelting", "gold_ore",    "gold_ingot",  2, 1, 1, 30000},
        {"Silver Smelt",  "silver_ore",  "silver_ingot",2, 1, 1, 30000},
        // 新路径：dust → ingot ×2 效率（GT:NH 经典处理链奖励）
        {"Steel From Dust",  "hematite_dust",  "steel",       2, 1, 2, 30000},
        {"Steel From Dust",  "magnetite_dust", "steel",       2, 1, 2, 30000},
        {"Aluminum From Dust","bauxite_dust",  "aluminum",    2, 1, 2, 30000},
        {"Tin From Dust",    "cassiterite_dust","tin",        2, 1, 2, 30000},
        {"Copper From Dust", "malachite_dust", "copper",      2, 1, 2, 30000},
        {"Copper From Dust", "chalcopyrite_dust","copper",    2, 1, 2, 30000},
        {"Gold From Dust",   "gold_dust",      "gold_ingot",  2, 1, 2, 30000},
        {"Silver From Dust", "silver_dust",    "silver_ingot",2, 1, 2, 30000}
    };

    int computeFrame(Slot& s) const {
        if (s.phase == IDLE) return 0;
        if (s.phase == DONE) return 23;
        int pct100 = s.totalMs == 0 ? 0 : (s.progressMs * 100) / s.totalMs;
        if (pct100 < 33) {
            return min(5, (pct100 * 6) / 33);
        } else if (pct100 < 83) {
            int seg = ((pct100 - 33) * 12) / 50;
            if (seg < 0) seg = 0; if (seg > 11) seg = 11;
            return 6 + seg;
        } else {
            int seg = ((pct100 - 83) * 6) / 17;
            if (seg < 0) seg = 0; if (seg > 5) seg = 5;
            return 18 + seg;
        }
    }

    void checkLevelUpInternal(PlayerData& player) {
        int needed = 100 + player.level * 20;
        while (player.exp >= needed) {
            player.exp -= needed;
            player.level++;
            logs.push_back("[F] Level Up! Current level: " + to_string(player.level));
            needed = 100 + player.level * 20;
        }
    }

public:
    BlastFurnace() : slots(4) {}

    int getSlotCount() const { return 4; }
    int getFocusedSlot() const { return focusedSlot; }
    void setFocusedSlot(int i) { if (i >= 0 && i < 4) focusedSlot = i; }
    Slot& getSlot(int i) { return slots[i]; }
    int getFrameForSlot(int i) {
        if (i < 0 || i >= 4) return 0;
        return computeFrame(slots[i]);
    }

    void addLog(const string& msg) {
        logs.push_back("[F" + to_string(focusedSlot) + "] " + msg);
        if (logs.size() > 30) logs.erase(logs.begin());
    }

    // ==== focused-slot 旧 API 兼容 ====
    string getStatus() {
        Slot& s = slots[focusedSlot];
        switch (s.phase) {
        case IDLE:    return "IDLE";
        case STOKING: return "STOKING";
        case HEATING: return "HEATING";
        case POURING: return "POURING";
        case DONE:    return "DONE (collect)";
        }
        return "IDLE";
    }
    int getProgress() {
        Slot& s = slots[focusedSlot];
        return s.totalMs == 0 ? 0 : (s.progressMs * 100) / s.totalMs;
    }
    int getMaxProgress() { return 100; }
    bool isActive() { Slot& s = slots[focusedSlot]; return s.phase != IDLE && s.phase != DONE; }
    bool isLoaded() { Slot& s = slots[focusedSlot]; return s.phase != IDLE; }
    int getTemperature() { return 1400; }
    int getTargetTemperature() { return 1400; }
    int getTemperatureTolerance() { return 75; }
    int getHeatTime() { return slots[focusedSlot].progressMs; }
    int getRequiredHeatTime() { return 30000; }

    void selectRecipe(int idx) {
        if (idx >= 0 && idx < (int)recipes.size()) selectedRecipe = idx;
    }
    int getSelectedRecipe() const { return selectedRecipe; }
    string getRecipeName() { return recipes[selectedRecipe].name; }
    string getRecipeInfo() {
        Recipe& r = recipes[selectedRecipe];
        return r.name + " (" + r.oreName + "): " + to_string(r.oreRequired) + " ore + " +
            to_string(r.fuelRequired) + " coal -> " + r.result + " x" +
            to_string(r.resultAmount) + "  [30s]";
    }
    vector<string> getRecipeList() {
        vector<string> res;
        for (auto& r : recipes)
            res.push_back(r.name + " [" + r.oreName + "] -> " + r.result +
                " x" + to_string(r.resultAmount) + " (30s)");
        return res;
    }
    int recipeCount() const { return (int)recipes.size(); }
    Recipe& getRecipe(int i) { return recipes[i]; }

    bool canLoad(PlayerData& player) {
        Slot& s = slots[focusedSlot];
        if (s.phase != IDLE && s.phase != DONE) return false;
        Recipe& r = recipes[selectedRecipe];
        return player.hasItem(r.oreName, r.oreRequired) &&
               player.hasItem("coal", r.fuelRequired);
    }
    int findRecipe(const string& oreName, int oreAmt, int fuelAmt) {
        for (int i = 0; i < (int)recipes.size(); ++i) {
            Recipe& r = recipes[i];
            if (r.oreName == oreName && r.oreRequired == oreAmt && r.fuelRequired == fuelAmt) return i;
        }
        return -1;
    }
    bool canLoad(PlayerData& player, const string& oreName, int oreAmt, int fuelAmt) {
        int idx = findRecipe(oreName, oreAmt, fuelAmt);
        Slot& s = slots[focusedSlot];
        return (s.phase == IDLE || s.phase == DONE) && idx >= 0 &&
               player.hasItem(oreName, oreAmt) && player.hasItem("coal", fuelAmt);
    }

    void loadMaterials(PlayerData& player) {
        Slot& s = slots[focusedSlot];
        if (s.phase != IDLE && s.phase != DONE) {
            addLog("X Slot is still in progress.");
            return;
        }
        Recipe& r = recipes[selectedRecipe];
        if (!canLoad(player)) {
            addLog("X NOT ENOUGH MATERIALS");
            return;
        }
        player.removeItem(r.oreName, r.oreRequired);
        player.removeItem("coal", r.fuelRequired);
        s.phase = STOKING;
        s.progressMs = 0;
        s.totalMs = r.durationMs;
        s.loadedRecipe = selectedRecipe;
        s.oreCount = r.oreRequired;
        s.fuelCount = r.fuelRequired;
        s.resultCount = r.resultAmount;
        s.recipeIndex = selectedRecipe;
        addLog("Loaded " + to_string(r.oreRequired) + " " + r.oreName +
               " + " + to_string(r.fuelRequired) + " coal. 30s burn begins.");
    }
    bool loadMaterials(PlayerData& player, const string& oreName, int oreAmt, int fuelAmt) {
        int idx = findRecipe(oreName, oreAmt, fuelAmt);
        if (idx < 0) return false;
        selectedRecipe = idx;
        loadMaterials(player);
        return true;
    }
    bool canStart(PlayerData& player) { return canLoad(player); }
    void start(PlayerData& player) { loadMaterials(player); }

    bool blowAir() { return false; }  // 土高炉：移除吹空气

    bool update(PlayerData& player, int elapsedMs = 100) {
        bool anyFinished = false;
        for (int i = 0; i < 4; ++i) {
            Slot& s = slots[i];
            if (s.phase == IDLE || s.phase == DONE) continue;
            s.progressMs += elapsedMs;
            int pct100 = s.totalMs == 0 ? 0 : (s.progressMs * 100) / s.totalMs;
            if (pct100 < 33) s.phase = STOKING;
            else if (pct100 < 83) s.phase = HEATING;
            else if (s.progressMs < s.totalMs) s.phase = POURING;
            else {
                if (s.loadedRecipe >= 0 && s.loadedRecipe < (int)recipes.size()) {
                    Recipe& r = recipes[s.loadedRecipe];
                    player.addItem(r.result, s.resultCount, "product", 20);
                    player.exp += 25;
                    logs.push_back("[F" + to_string(i) + "] OK! " + r.name +
                                   " -> " + r.result + " x" + to_string(s.resultCount));
                    checkLevelUpInternal(player);
                }
                s.phase = DONE;
                s.progressMs = s.totalMs;
                anyFinished = true;
            }
        }
        return anyFinished;
    }

    void cancel() {
        Slot& s = slots[focusedSlot];
        if (s.phase != IDLE) {
            s.phase = IDLE;
            s.progressMs = 0;
            s.loadedRecipe = -1;
            s.oreCount = 0; s.fuelCount = 0; s.resultCount = 0;
            addLog("Smelting in slot cancelled (materials lost).");
        }
    }

    void collectDone() {
        Slot& s = slots[focusedSlot];
        if (s.phase == DONE) { s.phase = IDLE; s.progressMs = 0; }
    }

    vector<string> getLogs() { return logs; }
};

// ======================== 车床系统 ========================
// 消耗 steel，按所选模具加工成机械件（gear/rod/plate/spring/bolt/wire）。
// 状态机 Idle->Inserting(5帧滑入)->Machining(进度+4帧循环动画)->Done。
// update() 一次同时推进进度与动画帧索引 —— 这是 FTXUI 内嵌动画的核心。
class Lathe {
public:
    enum AnimState { Idle, Inserting, Machining, Done };

private:
    AnimState animState = Idle;
    bool isRunning = false;
    bool loaded = false;
    int progress = 0;
    int maxProgress = 100;
    int accumulatedMs = 0;        // 当前 job 累计加工时长
    int selectedMold = 0;
    int loadedMold = 0;
    int frameIndex = 0;          // 每 tick +1，渲染时取模
    vector<string> logs;
    vector<vector<string>> insertFrames;     // 5 帧滑入动画
    vector<vector<string>> machiningFrames;  // 4 帧加工循环

    struct Mold {
        string name;            // "Gear" — Radiobox 显示
        string result;          // "gear" — addItem 名字
        int steelRequired;
        int resultAmount;
        int durationMs;
        int exp;
        int value;
        vector<string> iconArt; // 静态 ASCII 图标
    };
    vector<Mold> molds;

public:
    Lathe() {
        // 模具表（字段顺序: name, result, steel, amount, durationMs, exp, value）
        molds = {
            {"Gear",   "steel_gear",   1, 1, 3000, 12, 25, {}},
            {"Rod",    "steel_rod",    1, 1, 2500, 10, 20, {}},
            {"Plate",  "steel_plate",  1, 1, 2000,  8, 18, {}},
            {"Spring", "steel_spring", 1, 1, 3500, 14, 30, {}},
            {"Bolt",   "steel_bolt",   1, 2, 1500,  6, 12, {}},
            {"Wire",   "steel_wire",   1, 3, 4000, 16, 35, {}},
        };
        // 模具静态 icon（每个 6 行，纯基础 ASCII）
        molds[0].iconArt = { "   .---.", "  /  o  \\", " | o   o |", "  \\  o  /", "   '---'", "  [GEAR]" };
        molds[1].iconArt = { "   .----.", "  |      |", "  |      |", "  |      |", "   '----'", "  [ROD]" };
        molds[2].iconArt = { "  .======.", " |        |", " |        |", "  '======'", "", "  [PLATE]" };
        molds[3].iconArt = { "  /\\/\\/\\/", "  \\/\\/\\/", "  /\\/\\/\\/", "  \\/\\/\\/", "", "  [SPRING]" };
        molds[4].iconArt = { "   /|  |\\", "   \\|__|/", "   |====|", "   |    |", "   |____|", "  [BOLT]" };
        molds[5].iconArt = { "   .----.", "  |||||||", "   |    |", "  |||||||", "   '----'", "  [WIRE]" };

        // 插入动画 5 帧：钢条 [====] 从左滑入卡盘 |  |
        insertFrames = {
            { "[====]       |  |", "             |  |", "             |__|", "            /____\\", "           | LATHE |", "            \\____/" },
            { "   [====]    |  |", "             |  |", "             |__|", "            /____\\", "           | LATHE |", "            \\____/" },
            { "      [====]|  |", "             |  |", "             |__|", "            /____\\", "           | LATHE |", "            \\____/" },
            { "         [===|==]", "             |  |", "             |__|", "            /____\\", "           | LATHE |", "            \\____/" },
            { "             |==|", "             |  |", "             |__|", "            /____\\", "           | LATHE |", "            \\____/" },
        };
        // 加工动画 4 帧循环：铁屑 ~ 与火花 -- 位置变化（渲染时 frameIndex/2 % 4 → 200ms/帧）
        machiningFrames = {
            { "        |  |    ~", "        |  |  ~", "        |__|", "       /____\\", "      | LATHE |", "       \\____/" },
            { "   --   |  |    ~", "        |  |", "        |__|", "       /____\\", "      | LATHE |", "       \\____/" },
            { "        |  |  --  ~", "        |  |~", "        |__|", "       /____\\", "      | LATHE |", "       \\____/" },
            { "   ~    |  |  --", "        |  |", "        |__|", "       /____\\", "      | LATHE |", "       \\____/" },
        };

        addLog("Lathe ready. Pick a mold and load steel.");
    }

    void addLog(const string& msg) {
        logs.push_back(msg);
        if (logs.size() > 30) logs.erase(logs.begin());
    }

    string getStatus() {
        switch (animState) {
        case Inserting: return "inserting steel";
        case Machining: return "machining";
        case Done:      return "job complete";
        default:        return "free";
        }
    }

    int  getProgress() { return progress; }
    int  getMaxProgress() { return maxProgress; }
    bool isActive() { return animState == Machining || animState == Inserting; }
    bool isLoaded() { return loaded; }
    int  getSelectedMold() { return selectedMold; }
    AnimState getAnimState() { return animState; }
    bool getRunning() const { return isRunning; }
    // 每 tick 由 ChemicalWorldGame 先扣 EU 设置此字段，然后 update() 决定是否加工
    bool hasPowerThisTick = true;

    void selectMold(int index) {
        if (animState != Idle) return;
        if (index >= 0 && index < (int)molds.size()) selectedMold = index;
    }

    // 循环切换到下一个模具（SELECT 按钮用）
    void selectNextMold() {
        if (animState != Idle) return;
        selectedMold = (selectedMold + 1) % (int)molds.size();
    }

    bool canLoad(PlayerData& player) {
        if (animState != Idle) return false;
        return player.hasItem("steel", molds[selectedMold].steelRequired);
    }

    void loadMaterials(PlayerData& player) {
        if (animState != Idle) return;
        Mold& m = molds[selectedMold];
        if (!player.hasItem("steel", m.steelRequired)) {
            addLog("X Not enough steel for " + m.name);
            return;
        }
        player.removeItem("steel", m.steelRequired);
        loadedMold = selectedMold;
        animState = Inserting;
        frameIndex = 0;
        addLog("Loading " + to_string(m.steelRequired) + " steel into chuck...");
    }

    bool update(PlayerData& player, int elapsedMs = 100) {
        if (animState == Inserting) {
            frameIndex++;
            if (frameIndex >= (int)insertFrames.size()) {
                animState = Machining;
                isRunning = true;
                loaded = true;
                progress = 0;
                accumulatedMs = 0;
                frameIndex = 0;
                addLog("Steel locked. Machining " + molds[loadedMold].name + "...");
            }
            return false;
        }
        if (animState == Machining || animState == Inserting) {
            // 电力检测：世界先扣 EU 并设置 hasPowerThisTick；false 就暂停
            if (!hasPowerThisTick) {
                if (isRunning) {
                    isRunning = false;
                    addLog("X No power (need 2 EU/tick). Machining paused.");
                }
                return false;
            }
        }
        if (animState == Machining) {
            if (!isRunning) {
                isRunning = true;
                addLog("⚡ Power restored. Machining resumed.");
            }
            frameIndex++;
            accumulatedMs += elapsedMs;
            progress = min(maxProgress, accumulatedMs * maxProgress / molds[loadedMold].durationMs);
            if (accumulatedMs >= molds[loadedMold].durationMs) {
                Mold& m = molds[loadedMold];
                player.addItem(m.result, m.resultAmount, "product", m.value);
                player.exp += m.exp;
                addLog("OK! " + m.name + " done. Got " + m.result + " x" + to_string(m.resultAmount));
                // 升级检查
                int needed = 100 + player.level * 20;
                while (player.exp >= needed) {
                    player.exp -= needed;
                    player.level++;
                    addLog("Level Up! Current level: " + to_string(player.level));
                    needed = 100 + player.level * 20;
                }
                animState = Done;
                isRunning = false;
                loaded = false;
                progress = 100;
                frameIndex = 0;
                return true;
            }
            return false;
        }
        return false;
    }

    void cancel() {
        if (animState == Machining || animState == Inserting) {
            animState = Idle;
            isRunning = false;
            loaded = false;
            progress = 0;
            accumulatedMs = 0;
            frameIndex = 0;
            addLog("X Cancelled. Steel was not returned.");
        }
    }

    void collect() {
        if (animState == Done) {
            animState = Idle;
            progress = 0;
            accumulatedMs = 0;
            frameIndex = 0;
            addLog("Collected. Ready for next job.");
        }
    }

    vector<string> getLogs() { return logs; }

    vector<string> getMoldList() {
        vector<string> result;
        for (auto& m : molds) {
            result.push_back(m.name + " -> " + m.result + " x" + to_string(m.resultAmount));
        }
        return result;
    }

    string getMoldInfo() {
        Mold& m = molds[selectedMold];
        return m.name + ": " + to_string(m.steelRequired) + " steel -> " +
            m.result + " x" + to_string(m.resultAmount) + ", " +
            to_string(m.durationMs / 1000) + "." + to_string((m.durationMs / 100) % 10) + "s";
    }

    vector<string> getMoldArt(int index) {
        if (index < 0 || index >= (int)molds.size()) return {};
        return molds[index].iconArt;
    }

    // 按 animState 取当前动画帧；Idle 由调用方调 getMoldArt。
    vector<string> getCurrentFrame() {
        if (animState == Inserting) {
            int i = min(frameIndex, (int)insertFrames.size() - 1);
            return insertFrames[i];
        }
        if (animState == Machining) {
            return machiningFrames[(frameIndex / 2) % machiningFrames.size()];
        }
        if (animState == Done) {
            return molds[loadedMold].iconArt;  // 展示刚做好的产品
        }
        return {};
    }
};

// ======================== 矿物处理链机器 ========================
// Crusher 破碎机：4 槽并行，耗电 4 EU/tick/槽，1 ore → 2 crushed + 1 gravel
// 动画 12 帧：0-3 STONING(进料), 4-8 MILLING(齿轮旋转), 9-11 DONE_POUR(碎屑流出)
class Crusher {
public:
    enum SlotPhase { IDLE, STONING, MILLING, DONE_POUR };
    struct Slot {
        SlotPhase phase = IDLE;
        int recipeIndex = -1;
        int progressMs = 0;
        int totalMs = 10000;
        int loadedRecipe = -1;
        int oreCount = 0;
        int resultCount = 0;
    };
    struct Recipe {
        string name;
        string oreName;
        string crushedName;
        int oreRequired;
        int crushedAmount;
        int durationMs;
        string byproduct;
        int byproductAmount;
    };

private:
    vector<Slot> slots;
    int focusedSlot = 0;
    int selectedRecipe = 0;
    bool hasPowerThisTick[4];
    int frameIndex[4];
    vector<string> logs;

    vector<Recipe> recipes = {
        {"Crush Hematite",    "hematite",    "crushed_hematite",    1, 2, 10000, "gravel", 1},
        {"Crush Magnetite",   "magnetite",   "crushed_magnetite",   1, 2, 10000, "gravel", 1},
        {"Crush Bauxite",     "bauxite",     "crushed_bauxite",     1, 2, 10000, "gravel", 1},
        {"Crush Cassiterite", "cassiterite", "crushed_cassiterite", 1, 2, 10000, "gravel", 1},
        {"Crush Malachite",   "malachite",   "crushed_malachite",   1, 2, 10000, "gravel", 1},
        {"Crush Chalcopyrite","chalcopyrite","crushed_chalcopyrite",1, 2, 10000, "gravel", 1},
        {"Crush Gold Ore",    "gold_ore",    "crushed_gold",        1, 2, 10000, "gravel", 1},
        {"Crush Silver Ore",  "silver_ore",  "crushed_silver",      1, 2, 10000, "gravel", 1}
    };

    int computeFrame(const Slot& s) const {
        if (s.phase == IDLE) return 0;
        if (s.phase == DONE_POUR) return 11;
        int pct = s.totalMs > 0 ? (s.progressMs * 100) / s.totalMs : 0;
        if (pct < 33) return (pct * 4) / 33;            // 0-3 STONING
        if (pct < 83) return 4 + ((pct - 33) * 5) / 50; // 4-8 MILLING
        return 9 + min(2, ((pct - 83) * 3) / 17);       // 9-11 DONE_POUR
    }

public:
    Crusher() : slots(4) {
        for (int i = 0; i < 4; ++i) { hasPowerThisTick[i] = true; frameIndex[i] = 0; }
    }

    int getSlotCount() const { return (int)slots.size(); }
    Slot& getSlot(int i) { return slots[i]; }
    const Slot& getSlot(int i) const { return slots[i]; }
    void setSlotPower(int i, bool p) { if (i >= 0 && i < 4) hasPowerThisTick[i] = p; }
    bool getSlotPower(int i) const { return i >= 0 && i < 4 ? hasPowerThisTick[i] : true; }
    int getFrameIndex(int i) const { return i >= 0 && i < 4 ? frameIndex[i] : 0; }

    int getFocusedSlot() const { return focusedSlot; }
    void setFocusedSlot(int s) { if (s >= 0 && s < 4) focusedSlot = s; }
    int getSelectedRecipe() const { return selectedRecipe; }
    void setSelectedRecipe(int r) { if (r >= 0 && r < (int)recipes.size()) selectedRecipe = r; }
    int getRecipeCount() const { return (int)recipes.size(); }
    const Recipe& getRecipe(int i) const { return recipes[i]; }
    vector<string> getRecipeList() const {
        vector<string> v;
        for (auto& r : recipes) v.push_back(r.name + "  (" + r.oreName + " -> " + r.crushedName + " x" + to_string(r.crushedAmount) + ")");
        return v;
    }

    bool canLoad(PlayerData& p, int slotIdx) const {
        if (slotIdx < 0 || slotIdx >= 4) return false;
        if (slots[slotIdx].phase != IDLE) return false;
        const Recipe& r = recipes[selectedRecipe];
        return p.hasItem(r.oreName, r.oreRequired);
    }

    bool loadSlot(PlayerData& p, int slotIdx) {
        if (!canLoad(p, slotIdx)) return false;
        Recipe& r = recipes[selectedRecipe];
        p.removeItem(r.oreName, r.oreRequired);
        Slot& s = slots[slotIdx];
        s.phase = STONING;
        s.recipeIndex = selectedRecipe;
        s.loadedRecipe = selectedRecipe;
        s.oreCount = r.oreRequired;
        s.progressMs = 0;
        s.totalMs = r.durationMs;
        s.resultCount = 0;
        return true;
    }

    bool cancelSlot(int slotIdx) {
        if (slotIdx < 0 || slotIdx >= 4) return false;
        Slot& s = slots[slotIdx];
        if (s.phase == IDLE || s.phase == DONE_POUR) return false;
        s.phase = IDLE;
        s.progressMs = 0;
        return true;
    }

    bool collectSlot(PlayerData& p, int slotIdx) {
        if (slotIdx < 0 || slotIdx >= 4) return false;
        Slot& s = slots[slotIdx];
        if (s.phase != DONE_POUR) return false;
        Recipe& r = recipes[s.recipeIndex];
        p.addItem(r.crushedName, r.crushedAmount, "crushed", 8);
        p.addItem(r.byproduct, r.byproductAmount, "material", itemPrice(r.byproduct));
        s.phase = IDLE;
        s.progressMs = 0;
        s.oreCount = 0;
        s.resultCount = 0;
        return true;
    }

    void update(PlayerData& p, int elapsedMs = 100) {
        for (int i = 0; i < 4; ++i) {
            Slot& s = slots[i];
            if (s.phase == IDLE || s.phase == DONE_POUR) continue;
            if (!hasPowerThisTick[i]) continue;  // 无电暂停，不推进
            s.progressMs += elapsedMs;
            frameIndex[i] = (frameIndex[i] + 1) % 12;
            int pct = s.totalMs > 0 ? (s.progressMs * 100) / s.totalMs : 0;
            if (pct >= 33 && s.phase == STONING) s.phase = MILLING;
            if (pct >= 83 && s.phase == MILLING) s.phase = DONE_POUR;
            if (s.progressMs >= s.totalMs) {
                s.phase = DONE_POUR;
                s.resultCount = recipes[s.recipeIndex].crushedAmount;
            }
        }
    }

    int getProgressPct(int slotIdx) const {
        if (slotIdx < 0 || slotIdx >= 4) return 0;
        const Slot& s = slots[slotIdx];
        if (s.phase == IDLE) return 0;
        if (s.phase == DONE_POUR) return 100;
        return s.totalMs > 0 ? (s.progressMs * 100) / s.totalMs : 0;
    }
};

// OreWasher 洗矿槽：单槽，耗电 2 EU/tick，1 crushed → 1 purified + 1 sand + 30% 稀有
class OreWasher {
public:
    enum AnimState { Idle, Washing, Done };
    struct Recipe {
        string name;
        string crushedName;
        string purifiedName;
        string byproduct;
        int byproductAmount;
        int rareChance;        // 百分比 0-100
        string rareByproduct;
        int rareAmount;
        int durationMs;
    };

private:
    AnimState animState = Idle;
    bool isRunning = false;
    bool hasPowerThisTick = true;
    int progress = 0;
    int accumulatedMs = 0;
    int totalMs = 5000;
    int selectedRecipe = 0;
    int loadedRecipe = 0;
    int frameIndex = 0;
    string lastByproduct;
    string lastRareByproduct;
    int lastRareAmount = 0;

    vector<Recipe> recipes = {
        {"Wash Hematite",    "crushed_hematite",    "purified_hematite",    "sand", 1, 30, "copper_dust", 1, 5000},
        {"Wash Magnetite",   "crushed_magnetite",   "purified_magnetite",   "sand", 1, 30, "copper_dust", 1, 5000},
        {"Wash Bauxite",     "crushed_bauxite",     "purified_bauxite",     "sand", 1, 30, "rare_dust",   1, 5000},
        {"Wash Cassiterite", "crushed_cassiterite", "purified_cassiterite", "sand", 1, 30, "rare_dust",   1, 5000},
        {"Wash Malachite",   "crushed_malachite",   "purified_malachite",   "sand", 1, 40, "copper_dust", 1, 5000},
        {"Wash Chalcopyrite","crushed_chalcopyrite","purified_chalcopyrite","sand", 1, 40, "copper_dust", 1, 5000},
        {"Wash Gold",        "crushed_gold",        "purified_gold",        "sand", 1, 50, "rare_dust",   1, 5000},
        {"Wash Silver",      "crushed_silver",      "purified_silver",      "sand", 1, 50, "rare_dust",   1, 5000}
    };

public:
    AnimState getAnimState() const { return animState; }
    bool getRunning() const { return isRunning; }
    void setRunning(bool r) { isRunning = r; }
    bool getHasPowerThisTick() const { return hasPowerThisTick; }
    void setHasPowerThisTick(bool p) { hasPowerThisTick = p; }
    int getProgress() const { return progress; }
    int getSelectedRecipe() const { return selectedRecipe; }
    void setSelectedRecipe(int r) { if (r >= 0 && r < (int)recipes.size()) selectedRecipe = r; }
    int getRecipeCount() const { return (int)recipes.size(); }
    const Recipe& getRecipe(int i) const { return recipes[i]; }
    int getFrameIndex() const { return frameIndex; }
    string getLastByproduct() const { return lastByproduct; }
    string getLastRareByproduct() const { return lastRareByproduct; }
    int getLastRareAmount() const { return lastRareAmount; }

    vector<string> getRecipeList() const {
        vector<string> v;
        for (auto& r : recipes) {
            v.push_back(r.name + "  (" + r.crushedName + " -> " + r.purifiedName + ")");
        }
        return v;
    }

    bool canLoad(PlayerData& p) const {
        if (animState != Idle) return false;
        const Recipe& r = recipes[selectedRecipe];
        return p.hasItem(r.crushedName, 1);
    }

    bool loadMaterials(PlayerData& p) {
        if (!canLoad(p)) return false;
        Recipe& r = recipes[selectedRecipe];
        p.removeItem(r.crushedName, 1);
        animState = Washing;
        loadedRecipe = selectedRecipe;
        progress = 0;
        accumulatedMs = 0;
        totalMs = r.durationMs;
        isRunning = true;
        lastByproduct = "";
        lastRareByproduct = "";
        lastRareAmount = 0;
        return true;
    }

    bool collect(PlayerData& p) {
        if (animState != Done) return false;
        Recipe& r = recipes[loadedRecipe];
        p.addItem(r.purifiedName, 1, "purified", 12);
        p.addItem(r.byproduct, r.byproductAmount, "material", itemPrice(r.byproduct));
        if (lastRareAmount > 0 && !lastRareByproduct.empty()) {
            p.addItem(lastRareByproduct, lastRareAmount, "material", itemPrice(lastRareByproduct));
        }
        animState = Idle;
        progress = 0;
        accumulatedMs = 0;
        isRunning = false;
        return true;
    }

    void update(PlayerData& p, int elapsedMs = 100) {
        if (animState != Washing) return;
        if (!hasPowerThisTick) { isRunning = false; return; }
        isRunning = true;
        accumulatedMs += elapsedMs;
        frameIndex = (frameIndex + 1) % 8;
        progress = totalMs > 0 ? (accumulatedMs * 100) / totalMs : 0;
        if (accumulatedMs >= totalMs) {
            Recipe& r = recipes[loadedRecipe];
            lastByproduct = r.byproduct;
            // 判定稀有副产物
            if (r.rareChance > 0 && (rand() % 100) < r.rareChance) {
                lastRareByproduct = r.rareByproduct;
                lastRareAmount = r.rareAmount;
            } else {
                lastRareByproduct = "";
                lastRareAmount = 0;
            }
            animState = Done;
            progress = 100;
            isRunning = false;
        }
    }
};

// Centrifuge 离心机：单槽，耗电 8 EU/tick，1 purified → 1 dust + 50% 稀有
class Centrifuge {
public:
    enum AnimState { Idle, Spinning, Done };
    struct Recipe {
        string name;
        string purifiedName;
        string dustName;
        int rareChance;
        string rareByproduct;
        int rareAmount;
        int durationMs;
    };

private:
    AnimState animState = Idle;
    bool isRunning = false;
    bool hasPowerThisTick = true;
    int progress = 0;
    int accumulatedMs = 0;
    int totalMs = 15000;
    int selectedRecipe = 0;
    int loadedRecipe = 0;
    int frameIndex = 0;
    string lastRareByproduct;
    int lastRareAmount = 0;

    vector<Recipe> recipes = {
        {"Spin Hematite",    "purified_hematite",    "hematite_dust",    30, "copper_dust", 1, 15000},
        {"Spin Magnetite",   "purified_magnetite",   "magnetite_dust",   30, "copper_dust", 1, 15000},
        {"Spin Bauxite",     "purified_bauxite",     "bauxite_dust",     40, "rare_dust",   1, 15000},
        {"Spin Cassiterite", "purified_cassiterite", "cassiterite_dust", 40, "rare_dust",   1, 15000},
        {"Spin Malachite",   "purified_malachite",   "malachite_dust",   50, "copper_dust", 1, 15000},
        {"Spin Chalcopyrite","purified_chalcopyrite","chalcopyrite_dust",50, "copper_dust", 1, 15000},
        {"Spin Gold",        "purified_gold",        "gold_dust",        60, "rare_dust",   1, 15000},
        {"Spin Silver",      "purified_silver",      "silver_dust",      60, "rare_dust",   1, 15000}
    };

public:
    AnimState getAnimState() const { return animState; }
    bool getRunning() const { return isRunning; }
    void setRunning(bool r) { isRunning = r; }
    bool getHasPowerThisTick() const { return hasPowerThisTick; }
    void setHasPowerThisTick(bool p) { hasPowerThisTick = p; }
    int getProgress() const { return progress; }
    int getSelectedRecipe() const { return selectedRecipe; }
    void setSelectedRecipe(int r) { if (r >= 0 && r < (int)recipes.size()) selectedRecipe = r; }
    int getRecipeCount() const { return (int)recipes.size(); }
    const Recipe& getRecipe(int i) const { return recipes[i]; }
    int getFrameIndex() const { return frameIndex; }
    string getLastRareByproduct() const { return lastRareByproduct; }
    int getLastRareAmount() const { return lastRareAmount; }

    vector<string> getRecipeList() const {
        vector<string> v;
        for (auto& r : recipes) {
            v.push_back(r.name + "  (" + r.purifiedName + " -> " + r.dustName + ")");
        }
        return v;
    }

    bool canLoad(PlayerData& p) const {
        if (animState != Idle) return false;
        const Recipe& r = recipes[selectedRecipe];
        return p.hasItem(r.purifiedName, 1);
    }

    bool loadMaterials(PlayerData& p) {
        if (!canLoad(p)) return false;
        Recipe& r = recipes[selectedRecipe];
        p.removeItem(r.purifiedName, 1);
        animState = Spinning;
        loadedRecipe = selectedRecipe;
        progress = 0;
        accumulatedMs = 0;
        totalMs = r.durationMs;
        isRunning = true;
        lastRareByproduct = "";
        lastRareAmount = 0;
        return true;
    }

    bool collect(PlayerData& p) {
        if (animState != Done) return false;
        Recipe& r = recipes[loadedRecipe];
        p.addItem(r.dustName, 1, "dust", 18);
        if (lastRareAmount > 0 && !lastRareByproduct.empty()) {
            p.addItem(lastRareByproduct, lastRareAmount, "material", itemPrice(lastRareByproduct));
        }
        animState = Idle;
        progress = 0;
        accumulatedMs = 0;
        isRunning = false;
        return true;
    }

    void update(PlayerData& p, int elapsedMs = 100) {
        if (animState != Spinning) return;
        if (!hasPowerThisTick) { isRunning = false; return; }
        isRunning = true;
        accumulatedMs += elapsedMs;
        frameIndex = (frameIndex + 1) % 10;
        progress = totalMs > 0 ? (accumulatedMs * 100) / totalMs : 0;
        if (accumulatedMs >= totalMs) {
            Recipe& r = recipes[loadedRecipe];
            if (r.rareChance > 0 && (rand() % 100) < r.rareChance) {
                lastRareByproduct = r.rareByproduct;
                lastRareAmount = r.rareAmount;
            } else {
                lastRareByproduct = "";
                lastRareAmount = 0;
            }
            animState = Done;
            progress = 100;
            isRunning = false;
        }
    }
};

// ======================== 地图系统 ========================
struct Tile {
    char display;
    string name;
    string description;
    bool passable;
    int color;
    string mineral;
    int richness = 0;
    int hits = 0;
};

enum class Area { Home, Wasteland, Cave };

class GameMap {
private:
    int width = 60;
    int height = 50;
    Area area = Area::Home;
    vector<vector<Tile>> tiles;

    void fill(int w, int h, Tile tile) {
        width = w;
        height = h;
        tiles.assign(height, vector<Tile>(width, tile));
    }

    void placeMinerals(int count) {
        static const vector<pair<string, char>> minerals = {
            {"hematite", 'H'}, {"magnetite", 'M'}, {"bauxite", 'B'},
            {"coal", 'C'}, {"cassiterite", 'T'}, {"malachite", 'P'},
            {"chalcopyrite", 'U'}, {"gold_ore", 'G'}, {"silver_ore", 'S'}
        };
        for (int vein = 0; vein < count; ++vein) {
            int x = 2 + rand() % max(1, width - 4);
            int y = 2 + rand() % max(1, height - 4);
            auto mineral = minerals[rand() % minerals.size()];
            for (int i = 0; i < 2 + rand() % 4; ++i) {
                if (x > 0 && x < width - 1 && y > 0 && y < height - 1 &&
                    tiles[y][x].passable && tiles[y][x].mineral.empty()) {
                    tiles[y][x].display = mineral.second;
                    tiles[y][x].name = mineral.first;
                    tiles[y][x].description = "Exposed ore deposit";
                    tiles[y][x].mineral = mineral.first;
                    tiles[y][x].richness = 1 + rand() % 3;
                }
                if (rand() % 2) x += rand() % 3 - 1;
                else y += rand() % 3 - 1;
            }
        }
    }

public:
    GameMap() { generate(Area::Home); }

    void generate(Area newArea) {
        area = newArea;
        if (area == Area::Home) {
            fill(60, 50, { '.', "Grass", "Home grass", true, COLOR_GREEN });
            for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
                if (rand() % 14 == 0) tiles[y][x] = { 't', "Tree", "Decoration", false, COLOR_DARK_GREEN };
            // BlastFurnace 2×2（左上 (5,5)；大写主格红，辅格暗红，都不可踩）
            tiles[5][5] = { 'F', "Blast Furnace", "Installed machine", false, COLOR_RED };
            tiles[5][6] = { 'f', "Blast Furnace", "Installed machine", false, COLOR_DARK_RED };
            tiles[6][5] = { 'f', "Blast Furnace", "Installed machine", false, COLOR_DARK_RED };
            tiles[6][6] = { 'f', "Blast Furnace", "Installed machine", false, COLOR_DARK_RED };
            tiles[5][12] = { 'C', "Car", "Travel to the wasteland", true, COLOR_YELLOW };
            // Lathe 2×2（左上 (8,5)；紫/暗紫，都不可踩）
            tiles[8][5] = { 'L', "Lathe", "Machining facility", false, COLOR_PURPLE };
            tiles[8][6] = { 'l', "Lathe", "Machining facility", false, COLOR_DARK_PURPLE };
            tiles[9][5] = { 'l', "Lathe", "Machining facility", false, COLOR_DARK_PURPLE };
            tiles[9][6] = { 'l', "Lathe", "Machining facility", false, COLOR_DARK_PURPLE };
            // 玩家出生格 (10,5)，在车床右下方，确保不站在机器上
            tiles[10][5] = { '.', "Grass", "Home grass", true, COLOR_GREEN };

            // ===== 家园装饰 =====
            // 1) 河流：尝试几条不同方向的起点，找 >=10 连续格
            {
                int bestLen = 0;
                int bestX = 15, bestY = 38;
                int bestDir = 0;
                for (int attempt = 0; attempt < 5; ++attempt) {
                    int sx = 8 + rand() % (width - 12);
                    int sy = 28 + rand() % (height - 32);
                    for (int dir = 0; dir < 3; ++dir) {
                        int len = 0;
                        for (int step = 0; step < 15; ++step) {
                            int x = sx + (dir == 0 ? step : (dir == 1 ? 0 : step));
                            int y = sy + (dir == 1 ? step : (dir == 2 ? step : 0));
                            if (x < 0 || x >= width || y < 0 || y >= height) break;
                            if (tiles[y][x].display != '.') break;
                            len++;
                        }
                        if (len > bestLen) { bestLen = len; bestX = sx; bestY = sy; bestDir = dir; }
                    }
                }
                for (int step = 0; step < max(10, bestLen); ++step) {
                    int x = bestX + (bestDir == 0 ? step : (bestDir == 1 ? 0 : step));
                    int y = bestY + (bestDir == 1 ? step : (bestDir == 2 ? step : 0));
                    if (x >= 0 && x < width && y >= 0 && y < height && tiles[y][x].display == '.')
                        tiles[y][x] = { '~', "River", "Flowing water", false, COLOR_BLUE };
                }
            }
            // 2) 湖泊：3×5 固定尺寸，位置在右下角远离开机器/出生
            {
                int tries = 0;
                while (tries++ < 50) {
                    int cx = 40 + rand() % 10;
                    int cy = 32 + rand() % 8;
                    bool ok = true;
                    for (int dy = -1; dy <= 1; ++dy) for (int dx = -2; dx <= 2; ++dx) {
                        int x = cx + dx, y = cy + dy;
                        if (x < 0 || x >= width || y < 0 || y >= height) ok = false;
                        else if (tiles[y][x].display != '.') ok = false;
                    }
                    if (ok) {
                        for (int dy = -1; dy <= 1; ++dy) for (int dx = -2; dx <= 2; ++dx) {
                            tiles[cy + dy][cx + dx] = { '~', "Lake", "Calm water", false, COLOR_DARK_CYAN };
                        }
                        break;
                    }
                }
            }
            // 3) 花：随机 40 朵，3 种颜色
            {
                int placed = 0, tries = 0;
                while (placed < 40 && tries++ < 2000) {
                    int x = rand() % width;
                    int y = rand() % height;
                    if (tiles[y][x].display != '.') continue;
                    int c = COLOR_PURPLE;
                    int r = rand() % 3;
                    if (r == 0) c = COLOR_YELLOW;
                    else if (r == 1) c = COLOR_GREEN;
                    tiles[y][x] = { '*', "Flower", "Wild flower", true, c };
                    placed++;
                }
            }
            // 4) 草丛：随机 30 丛
            {
                int placed = 0, tries = 0;
                while (placed < 30 && tries++ < 1500) {
                    int x = rand() % width;
                    int y = rand() % height;
                    if (tiles[y][x].display != '.') continue;
                    tiles[y][x] = { 'v', "Grass tuft", "Tall grass", true, COLOR_DARK_GREEN };
                    placed++;
                }
            }
        }
        else if (area == Area::Wasteland) {
            fill(150, 80, { '.', "Wasteland", "Dry ground", true, COLOR_DARK_YELLOW });
            for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
                if (rand() % 11 == 0) tiles[y][x] = { '=', "Rock", "Impassable rock", false, COLOR_GREY };
                else if (rand() % 17 == 0) tiles[y][x] = { '~', "Salt Flats", "Pale biome", true, COLOR_LIGHT_WHITE };
                else if (rand() % 19 == 0) tiles[y][x] = { '^', "Red Dunes", "Red dune biome", true, COLOR_RED };
            }
            tiles[2][2] = { 'C', "Car", "Return to home", true, COLOR_YELLOW };
            tiles[height / 2][width / 2] = { 'O', "Cave Entrance", "Enter cave", true, COLOR_CYAN };
            placeMinerals(90);
        }
        else {
            fill(45, 28, { '.', "Cave Floor", "Cave floor", true, COLOR_GREY });
            for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
                if (x == 0 || y == 0 || x == width - 1 || y == height - 1 || rand() % 7 == 0)
                    tiles[y][x] = { '=', "Rock", "Impassable rock", false, COLOR_GREY };
            tiles[1][1] = { 'O', "Cave Exit", "Return to wasteland", true, COLOR_CYAN };
            placeMinerals(35);
        }
    }

    Area getArea() const { return area; }
    bool isNear(int x, int y, char symbol, int px, int py) const {
        return abs(x - px) + abs(y - py) <= 1 && getTile(x, y).display == symbol;
    }

    Tile& getTile(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            static Tile wall = { '#', "Boundary", "World boundary", false, COLOR_RED };
            return wall;
        }
        return tiles[y][x];
    }

    const Tile& getTile(int x, int y) const {
        static const Tile wall = { '#', "Boundary", "World boundary", false, COLOR_RED };
        if (x < 0 || x >= width || y < 0 || y >= height) return wall;
        return tiles[y][x];
    }

    bool isPassable(int x, int y) {
        return getTile(x, y).passable;
    }

    bool mineAt(PlayerData& player, int x, int y) {
        Tile& tile = getTile(x, y);
        // No mineral here
        if (tile.mineral.empty()) return false;

        tile.hits++;
        cout << " You strike the " << tile.mineral << " (" << tile.hits << "/3)." << endl;
        if (tile.hits == 2) {
            cout << " Cracks spread across the ore." << endl;
        }

        if (tile.hits >= 3) {
            int amount = (tile.richness == 1) ? 1 : (tile.richness == 2 ? 2 : 4);
            player.addItem(tile.mineral, amount, "ore", 5);
            cout << " Mined " << tile.mineral << " x" << amount << "!" << endl;

            // Reset tile to empty ground
            tile.mineral.clear();
            tile.display = '.';
            tile.name = "Stone";
            tile.description = "Empty ground";
            tile.hits = 0;
            tile.richness = 0;
        }
        return true;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    char getTileDisplay(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return '#';
        return tiles[y][x].display;
    }

    void interact(PlayerData& player, int x, int y) {
        Tile& tile = getTile(x, y);
        cout << (tile.mineral.empty() ? " Nothing to interact with." :
            " Use E near this deposit to mine it.") << endl;
    }

    void save(ostream& output) const {
        output << width << ' ' << height << '\n';
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const Tile& tile = tiles[y][x];
                output << tile.display << ' ' << (tile.mineral.empty() ? "-" : tile.mineral) << ' ' << tile.richness << ' '
                    << tile.hits << ' ' << tile.color << '\n';
            }
        }
    }

    bool load(istream& input) {
        int width = 0;
        int height = 0;
        if (!(input >> width >> height) || width <= 0 || height <= 0) return false;
        tiles.assign(height, vector<Tile>(width));
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Tile& tile = tiles[y][x];
                string mineral;
                if (!(input >> tile.display >> mineral >> tile.richness >> tile.hits >> tile.color)) return false;
                tile.mineral = mineral == "-" ? "" : mineral;
                tile.passable = tile.display != '=' && tile.display != 't'
                    && tile.display != 'F' && tile.display != 'L'
                    && tile.display != 'f' && tile.display != 'l'
                    && tile.display != 'G' && tile.display != 'g'
                    && tile.display != '+';
                tile.name = tile.mineral.empty() ? "Stone" : tile.mineral;
                tile.description = tile.mineral.empty() ? "Empty ground" : "Mineral deposit";
            }
        }
        return true;
    }
};

// ======================== FTXUI 高炉界面 ========================
class FurnaceUI {
private:
    BlastFurnace& furnace;
    PlayerData& player;
    ScreenInteractive& screen;
    bool running = true;
    string statusMessage = "WELCOME! Earth blast furnace: 4-slot parallel x 30s each. No power needed.";

    Component mainContainer;
    Component recipeList;
    Component slotTabs;
    Component buttonLoad;
    Component buttonCancel;
    Component buttonCollect;
    Component buttonClose;
    Component logViewer;
    Component progressRenderer;

    vector<string> recipeNames;
    vector<string> slotTabLabels;
    int focusedSlotLocal = 0;
    int selRecLocal = 0;

public:
    FurnaceUI(BlastFurnace& f, PlayerData& p, ScreenInteractive& s)
        : furnace(f), player(p), screen(s) {
        setupUI();
    }

    // 返回 24 帧的单帧 ASCII 画面，11 行文本
    vector<string> artFrame(int frame) {
        if (frame < 0) frame = 0;
        if (frame > 23) frame = 23;
        string fire;
        Color col = Color::YellowLight;
        if (frame <= 5) {
            // STOKING 0-5：工人铲煤入炉 + 零星火苗
            const char* f0 = "        ";
            const char* f1 = "    \\\\  /";
            const char* f2 = "     >#<";
            const char* f3 = "    /[\\\\";
            switch (frame) {
            case 0: fire = string(f0); break;
            case 1: fire = "    \\\\   "; break;
            case 2: fire = string(f1); break;
            case 3: fire = string(f2); break;
            case 4: fire = string(f3); break;
            case 5: fire = "   \\\\#_/ "; break;
            }
        } else if (frame <= 17) {
            // HEATING 6-17：火焰高度随 frame 增大，12 帧渐强
            int level = frame - 6;      // 0..11
            string base = "   ";
            for (int i = 0; i < 3 + level / 2; ++i) base += "|^^| ";
            if (level >= 6) base = "   \\/\\/\\/\\/\\/\\/";
            fire = base.substr(0, 11);
            col = Color::RedLight;
        } else {
            // POURING 18-23：液态金属流出
            int stage = frame - 18; // 0..5
            const char* arr[] = {
                "     |==| ",
                "     |==\\",
                "     |==\\\\",
                "      \\==\\\\",
                "       \\==\\\\_",
                "        \\===\\\\__"
            };
            fire = arr[stage];
            col = Color::YellowLight;
        }

        vector<string> art = {
            "  +-------------------+",
            "  |  EARTH BLAST FURN.|",
            "  |                   |",
            "  |    (o)    (o)     |",
            "  |    |______|       |",
            "  |   /        \\      |",
            string("  |  ") + fire + string(string::size_type(19 - 3 - fire.size()), ' ') + string("|"),
            "  |   \\________/      |",
            "  |                   |",
            "  +-------------------+",
            "   frame = " + to_string(frame) + " / 23"
        };
        return art;
    }

    void setupUI() {
        // 配方（Radiobox 使用 oreName 列表显示选择）
        recipeNames.clear();
        for (int i = 0; i < furnace.recipeCount(); ++i) {
            auto& r = furnace.getRecipe(i);
            recipeNames.push_back(r.oreName + " (" + to_string(r.oreRequired) + "+c" +
                                   to_string(r.fuelRequired) + " -> " + r.result + ")");
        }
        int selRec = furnace.getSelectedRecipe();
        if (selRec < 0) selRec = 0;
        if (selRec >= (int)recipeNames.size()) selRec = 0;
        selRecLocal = selRec;
        recipeList = Menu(&recipeNames, &selRecLocal);
        recipeList = Renderer(recipeList, [&] {
            furnace.selectRecipe(selRecLocal);
            return recipeList->Render();
        });

        // 4 个槽位 Tab
        slotTabLabels = { " Slot 0 "," Slot 1 "," Slot 2 "," Slot 3 " };
        focusedSlotLocal = furnace.getFocusedSlot();
        slotTabs = Radiobox(&slotTabLabels, &focusedSlotLocal);
        slotTabs = Renderer(slotTabs, [&] {
            furnace.setFocusedSlot(focusedSlotLocal);
            return slotTabs->Render();
        });

        buttonLoad = Button("LOAD SLOT", [&] {
            furnace.setFocusedSlot(focusedSlotLocal);
            if (furnace.canLoad(player)) {
                furnace.loadMaterials(player);
                statusMessage = "Slot " + to_string(focusedSlotLocal) + " loaded. 30s timer started.";
            } else {
                statusMessage = "Need recipe's ore + coal (check slot is idle).";
            }
        });
        buttonCancel = Button("CANCEL SLOT", [&] {
            furnace.setFocusedSlot(focusedSlotLocal);
            furnace.cancel();
            statusMessage = "Slot " + to_string(focusedSlotLocal) + " cancelled.";
        });
        buttonCollect = Button("COLLECT SLOT", [&] {
            furnace.setFocusedSlot(focusedSlotLocal);
            auto& s = furnace.getSlot(focusedSlotLocal);
            if (s.phase == BlastFurnace::DONE) {
                furnace.collectDone();
                statusMessage = "Slot " + to_string(focusedSlotLocal) + " cleared. Ready for new batch.";
            } else {
                statusMessage = "Slot is not DONE yet.";
            }
        });
        buttonClose = Button("CLOSE", [&] {
            running = false;
            screen.ExitLoopClosure()();
        });

        progressRenderer = Renderer([&] {
            furnace.setFocusedSlot(focusedSlotLocal);
            Elements allBars;
            for (int i = 0; i < 4; ++i) {
                auto& s = furnace.getSlot(i);
                string lbl = "[" + to_string(i) + "] ";
                if (s.phase == BlastFurnace::IDLE) lbl += "IDLE          ";
                else if (s.phase == BlastFurnace::DONE) lbl += "DONE          ";
                else {
                    int pct = s.totalMs == 0 ? 0 : (s.progressMs * 100) / s.totalMs;
                    lbl += (s.phase == BlastFurnace::STOKING ? "STOKE " :
                            s.phase == BlastFurnace::HEATING ? "HEAT  " : "POUR  ");
                    lbl += to_string(pct) + "%";
                }
                string bar(30, '.');
                if (s.phase != BlastFurnace::IDLE && s.phase != BlastFurnace::DONE) {
                    int pct = s.totalMs == 0 ? 0 : (s.progressMs * 30) / s.totalMs;
                    for (int k = 0; k < pct && k < 30; ++k) bar[k] = '#';
                } else if (s.phase == BlastFurnace::DONE) {
                    bar.assign(30, '*');
                }
                Color c = Color::GrayDark;
                if (s.phase == BlastFurnace::STOKING) c = Color::YellowLight;
                else if (s.phase == BlastFurnace::HEATING) c = Color::RedLight;
                else if (s.phase == BlastFurnace::POURING) c = Color::MagentaLight;
                else if (s.phase == BlastFurnace::DONE) c = Color::Green;
                allBars.push_back(hbox({
                    text(lbl) | size(WIDTH, EQUAL, 20),
                    text(" [" + bar + "] ") | color(c),
                }));
            }
            return vbox(std::move(allBars)) | border;
        });

        logViewer = Renderer([&] {
            auto logs = furnace.getLogs();
            Elements elems;
            for (auto& l : logs) elems.push_back(text(l));
            if (elems.empty()) elems.push_back(text("(no logs)"));
            return vbox(elems) | border | size(HEIGHT, LESS_THAN, 8);
        });

        auto layout = Container::Vertical({
            slotTabs, recipeList,
            buttonLoad, buttonCancel, buttonCollect, buttonClose,
        });

        mainContainer = Renderer(layout, [&] {
            furnace.setFocusedSlot(focusedSlotLocal);
            int frame = furnace.getFrameForSlot(focusedSlotLocal);
            vector<string> art = artFrame(frame);
            Elements artLines;
            for (auto& ln : art) artLines.push_back(text(ln) | color(Color::YellowLight));
            auto artBox = vbox(std::move(artLines)) | borderDouble;

            auto& r = furnace.getRecipe(furnace.getSelectedRecipe());
            string recipeInfo = r.name + " (" + r.oreName + " x" + to_string(r.oreRequired) +
                                " + coal x" + to_string(r.fuelRequired) + ") -> " + r.result + " x" +
                                to_string(r.resultAmount) + "  [30s]";
            int oreCnt = player.getItemCount(r.oreName);
            int coalCnt = player.getItemCount("coal");
            int steelCnt = player.getItemCount("steel");

            auto status = furnace.getStatus();
            Color statusCol = (status == "IDLE" || status == "DONE (collect)")
                                ? Color::GrayDark : Color::Cyan;

            return vbox({
                text("        EARTH BLAST FURNACE  //  4-SLOT CONTROL") | bold | color(Color::Yellow),
                separator(),
                slotTabs->Render() | center,
                separator(),
                hbox({
                    vbox({
                        text(" RECIPES") | bold | color(Color::Cyan), separator(),
                        recipeList->Render() | flex,
                    }) | size(WIDTH, GREATER_THAN, 50) | flex,
                    vbox({
                        text(" FURNACE ART (24-frame loop)") | bold | color(Color::Cyan),
                        separator(),
                        artBox | center,
                        separator(),
                        progressRenderer->Render(),
                    }) | size(WIDTH, EQUAL, 54),
                }),
                separator(),
                hbox({
                    text(" Status: ") | bold, text(status) | color(statusCol), filler(),
                    text(" Recipe: ") | bold, text(recipeInfo) | color(Color::Green),
                }),
                hbox({
                    text(" Bag: ") | bold,
                    text(r.oreName + " x" + to_string(oreCnt)) | color(Color::RedLight),
                    text("   coal x" + to_string(coalCnt)) | color(Color::Yellow),
                    text("   steel x" + to_string(steelCnt)) | color(Color::Cyan),
                }),
                separator(),
                hbox({
                    buttonLoad->Render() | flex,
                    buttonCollect->Render() | flex,
                    buttonCancel->Render() | flex,
                    buttonClose->Render() | flex,
                }),
                separator(),
                text(" EVENT LOG") | bold | color(Color::Yellow),
                logViewer->Render() | flex,
                text(statusMessage) | color(Color::Green),
                text("Tip: 4 slots run in parallel. DONE batches auto-deliver product. COLLECT to clear DONE.")
                    | color(Color::GrayDark),
            }) | border | size(WIDTH, GREATER_THAN, 106);
        });

        // Event tick：openFurnaceUI 里的 thread 先调过 globalTick100ms，
        // 这里只需要标记刷新即可（furnace.update 在 globalTick 内已经执行）。
        mainContainer = mainContainer | CatchEvent([&](Event e) {
            if (e == Event::Custom) { return true; }
            return false;
        });
    }

    Component getComponent() { return mainContainer; }
    bool isRunning() { return running; }
    void refresh() {}
};

// ======================== 车床 FTXUI 界面 ========================
// 镜像 FurnaceUI：Maybe 门控按钮、CatchEvent(Event::Custom) 驱动 lathe.update()，
// artBox 在 Idle 显示模具 icon、其他状态显示动画帧。
class LatheUI {
private:
    Lathe& lathe;
    PlayerData& player;
    ScreenInteractive& screen;
    bool running = true;
    string statusMessage = "WELCOME TO THE LATHE! Requires power (2 EU/tick while machining).";

    Component mainContainer;
    Component moldList;
    Component buttonLoad;
    Component buttonSelect;
    Component buttonCancel;
    Component buttonCollect;
    Component buttonClose;
    Component progressBar;
    Component logViewer;
    Component artBox;

    std::vector<std::string> moldNames;
    int selectedInput = 0;
    bool needRefresh = false;

public:
    LatheUI(Lathe& l, PlayerData& p, ScreenInteractive& s)
        : lathe(l), player(p), screen(s) {
        setupUI();
    }

    void setupUI() {
        moldNames = lathe.getMoldList();
        moldList = Radiobox(&moldNames, &selectedInput);

        // 切换模具时同步给 lathe 并刷新 artBox（只拦方向键，Event::Custom fallthrough）
        moldList |= CatchEvent([&](Event event) {
            if (event == Event::ArrowUp || event == Event::ArrowDown) {
                lathe.selectMold(selectedInput);
                needRefresh = true;
                return true;
            }
            return false;
            });

        buttonLoad = Button("LOAD STEEL", [&] {
            if (lathe.canLoad(player)) {
                lathe.loadMaterials(player);
                statusMessage = "Steel sliding in...";
            }
            else {
                statusMessage = "Need steel for this mold.";
            }
            needRefresh = true;
            });

        // SELECT：循环切换到下一个模具，artBox 同步显示新模具 icon/动画
        buttonSelect = Button("SELECT", [&] {
            lathe.selectNextMold();
            selectedInput = lathe.getSelectedMold();  // 同步给 Radiobox 高亮
            statusMessage = "Mold: " + lathe.getMoldInfo();
            needRefresh = true;
            });

        buttonCancel = Button("CANCEL", [&] {
            lathe.cancel();
            statusMessage = "Cancelled.";
            needRefresh = true;
            });

        buttonCollect = Button("COLLECT", [&] {
            lathe.collect();
            statusMessage = "Ready for next job.";
            needRefresh = true;
            });

        buttonClose = Button("CLOSE", [&] {
            running = false;
            screen.ExitLoopClosure()();
            });

        // 进度条：暂停时灰色
        progressBar = Renderer([&] {
            int prog = lathe.getProgress();
            int maxProg = lathe.getMaxProgress();
            string bar = "[";
            for (int i = 0; i < 30; i++) {
                if (i < (prog * 30) / maxProg) bar += "#";
                else bar += ".";
            }
            bar += "]";
            string percent = to_string(prog) + "%";
            bool paused = (lathe.getAnimState() == Lathe::Machining ||
                           lathe.getAnimState() == Lathe::Inserting) && !lathe.getRunning();
            Color col = paused ? Color::GrayLight : Color::Green;
            return hbox({ text("  "), text(bar) | color(col),
                text("  "), text(percent),
                paused ? text("   ! PAUSED (no power)") | color(Color::Yellow)
                       : filler() });
            });

        // 日志查看器
        logViewer = Renderer([&] {
            auto logs = lathe.getLogs();
            Elements elems;
            for (auto& log : logs) {
                elems.push_back(text(log));
            }
            if (elems.empty()) {
                elems.push_back(text("(no logs)"));
            }
            return vbox(elems) | border | size(HEIGHT, LESS_THAN, 8);
            });

        // ASCII 动画/icon 渲染区（Idle 显示选中模具 icon，其他状态显示动画帧）
        artBox = Renderer([&] {
            vector<string> frame;
            if (lathe.getAnimState() == Lathe::Idle) {
                frame = lathe.getMoldArt(lathe.getSelectedMold());
            }
            else {
                frame = lathe.getCurrentFrame();
            }
            Elements lines;
            for (auto& line : frame) {
                lines.push_back(text(line));
            }
            return vbox(lines) | border | size(WIDTH, GREATER_THAN, 28) | size(HEIGHT, GREATER_THAN, 10);
            });

        // 视图状态门控：避免 Tab 误触不可见按钮（put-button bug 的教训）
        auto showIdle = [this] { return lathe.getAnimState() == Lathe::Idle; };
        auto showMachining = [this] { return lathe.getAnimState() == Lathe::Machining; };
        auto showDone = [this] { return lathe.getAnimState() == Lathe::Done; };

        auto layout = Container::Vertical({
            Container::Horizontal({
                Maybe(moldList, showIdle),
                Container::Vertical({
                    Maybe(buttonLoad,    showIdle),
                    Maybe(buttonSelect,  showIdle),
                    Maybe(buttonCancel,  showMachining),
                    Maybe(buttonCollect, showDone),
                    Maybe(buttonClose,   [] { return true; }),
                }),
            }),
            artBox,
            progressBar,
            logViewer,
            });

        mainContainer = Renderer(layout, [&] {
            int steel = player.getItemCount("steel");

            if (lathe.getAnimState() == Lathe::Inserting) {
                return vbox({
                    text("        LATHE  //  INSERTING STEEL") | bold | color(Color::Magenta),
                    separator(),
                    artBox->Render(),
                    text(statusMessage) | color(Color::Yellow),
                    }) | border | size(WIDTH, GREATER_THAN, 60);
            }
            if (lathe.getAnimState() == Lathe::Machining) {
                bool paused = !lathe.getRunning();
                Elements eInfo;
                eInfo.push_back(hbox({ text(" Status:   ") | bold,
                    text("MACHINING (2 EU/tick)") | color(Color::Cyan) }));
                eInfo.push_back(hbox({ text(" Mold:     ") | bold,
                    text(lathe.getMoldInfo()) | color(Color::Green) }));
                if (paused) {
                    eInfo.push_back(text("! PAUSED: No power. Feed coal at thermal generators!")
                                    | color(Color::Yellow) | bold);
                }
                return vbox({
                    text("        LATHE  //  MACHINING") | bold | color(Color::Red),
                    separator(),
                    vbox(std::move(eInfo)) | border,
                    separator(),
                    artBox->Render(),
                    progressBar->Render() | border,
                    hbox({ buttonCancel->Render() | flex }),
                    text(statusMessage) | color(Color::Yellow),
                    }) | border | size(WIDTH, GREATER_THAN, 78);
            }
            if (lathe.getAnimState() == Lathe::Done) {
                return vbox({
                    text("        LATHE  //  JOB COMPLETE") | bold | color(Color::Green),
                    separator(),
                    artBox->Render(),
                    text(statusMessage) | color(Color::Green),
                    hbox({ buttonCollect->Render() | flex, buttonClose->Render() | flex }),
                    }) | border | size(WIDTH, GREATER_THAN, 60);
            }

            // Idle 主视图
            return vbox({
                text("        LATHE  //  CONTROL DECK") | bold | color(Color::Yellow),
                separator(),
                hbox({ text(" Status:     ") | bold, text(lathe.getStatus()) | color(Color::Cyan) }),
                hbox({ text(" Mold:       ") | bold, text(lathe.getMoldInfo()) | color(Color::Green) }),
                hbox({ text(" Power:      ") | bold,
                       text("(see Generator panel via E near a generator)") | color(Color::GrayDark) }),
                hbox({ text(" STEEL:      ") | bold | color(Color::Magenta),
                       text("x" + to_string(steel)) | color(Color::Cyan) }),
                separator(),
                artBox->Render(),
                separator(),
                hbox({ buttonLoad->Render() | flex, buttonSelect->Render() | flex, buttonClose->Render() | flex }),
                separator(),
                text(" EVENT LOG") | bold | color(Color::Yellow),
                logViewer->Render() | flex,
                text(statusMessage) | color(Color::Yellow),
                text("Tip: 2 EU/tick consumed during machining. No power ⇒ machining pauses.") |
                    color(Color::GrayDark),
                }) | border | size(WIDTH, GREATER_THAN, 82);
            });

        // tick 钩子：openLatheUI 里的 thread 每 100ms 先调 globalTick100ms()
        // （扣电 + 状态推进），然后再发 Event::Custom 到这里 —— 这里只刷新 UI。
        mainContainer |= CatchEvent([&](Event event) {
            if (event == Event::Custom) {
                if (lathe.getAnimState() == Lathe::Done) {
                    statusMessage = "Machining complete! Press COLLECT.";
                }
                needRefresh = true;
                return true;
            }
            return false;
        });
    }

    Component getComponent() { return mainContainer; }
    bool isRunning() { return running; }

    void refresh() {
        if (needRefresh) {
            needRefresh = false;
        }
    }
};

// ======================== 交易 FTXUI 界面 ========================
// 图形化交易界面：左侧列表 + 右侧详情
// Tab = SELL（原功能） / BUY（原材料/钢零件/蓝图购买，买入价=itemPrice*3；蓝图只买 1 次）
class TradeUI {
private:
    PlayerData& player;
    ScreenInteractive& screen;
    bool running = true;
    string statusMessage = "Welcome to the market!";

    Component mainContainer;
    Component menu;
    Component buttonSellOne;
    Component buttonSellAll;
    Component buttonSellEverything;
    Component buttonBuyOne;
    Component buttonClose;
    Component detailBox;
    Component tabBar;

    int tabSelected = 0;   // 0 = SELL, 1 = BUY
    int lastTab = -1;      // 用于检测 tabBar 变化时触发 rebuildCurrentTab
    int selected = 0;
    vector<string> tabLabels;

    // ===== SELL tab data =====
    vector<string> sellEntries;
    vector<string> sellNames;
    vector<int>    sellQty;
    vector<int>    sellPrices;

    // ===== BUY tab data =====
    // 固定条目：原材料 + 钢零件 + 蓝图（2 项）
    struct BuyEntry {
        string label;       // Radiobox 文本
        string itemName;    // 物品名（蓝图留空，看 isBlueprint）
        int buyPrice;       // 买入价（c）
        bool isBlueprint = false;
        bool genBlueprint = false;  // true=发电机蓝图 false=电线蓝图
    };
    vector<BuyEntry> buyEntries;
    // buyLabels 必须是类成员：Menu 组件和 Renderer 都要持有它的指针/引用，
    // 局部变量会在 setupUI() 返回后析构导致悬空引用（点 BUY 1 重渲染即崩）。
    vector<string> buyLabels;

    bool& genBPUnlocked;
    bool& wireBPUnlocked;

    void rebuildSell() {
        sellEntries.clear(); sellNames.clear(); sellQty.clear(); sellPrices.clear();
        for (auto& item : player.inventory) {
            if (item.quantity > 0) {
                int p = itemPrice(item.name);
                sellNames.push_back(item.name);
                sellQty.push_back(item.quantity);
                sellPrices.push_back(p);
                string line = "  " + item.name + "  x" + to_string(item.quantity);
                line += p > 0 ? ("   [" + to_string(p) + "c]") : "   [not sellable]";
                sellEntries.push_back(line);
            }
        }
        if (sellEntries.empty()) {
            sellEntries.push_back("  (backpack empty)");
            sellNames.push_back(""); sellQty.push_back(0); sellPrices.push_back(0);
        }
    }

    void rebuildBuy() {
        buyEntries.clear();
        // 1. 原材料 (itemPrice * 3 向上取整为整数)
        vector<string> rawNames = { "hematite","magnetite","bauxite","cassiterite","malachite",
                                     "chalcopyrite","gold_ore","silver_ore","coal","sand","glass",
                                     "steel","iron_ingot","alloy" };
        for (auto& n : rawNames) {
            int base = itemPrice(n);
            int bp = base * 3;
            if (bp <= 0) bp = 10;
            buyEntries.push_back({ "  " + n + "   [" + to_string(bp) + "c]   x1", n, bp, false, false });
        }
        // 2. 钢零件
        vector<string> partNames = { "steel_gear","steel_rod","steel_plate","steel_spring",
                                      "steel_bolt","steel_wire" };
        for (auto& n : partNames) {
            int base = itemPrice(n);
            int bp = max(base * 3, 30);
            buyEntries.push_back({ "  " + n + "   [" + to_string(bp) + "c]   x1", n, bp, false, false });
        }
        // 3. 蓝图（底价 150 / 50）
        {
            string label = genBPUnlocked ? "  [OWNED] Generator Blueprint (150c)"
                                          : "  Generator Blueprint   [150c]";
            buyEntries.push_back({ label, "", 150, true, true });
        }
        {
            string label = wireBPUnlocked ? "  [OWNED] Wire Blueprint (50c)"
                                          : "  Wire Blueprint   [50c]";
            buyEntries.push_back({ label, "", 50, true, false });
        }
    }

    // 由 buyEntries 同步出 menu 用的标签 vector。必须是成员函数：lambda 捕获会随
    // setupUI 返回而悬空。
    void genBuyLabels() {
        rebuildBuy();
        buyLabels.clear();
        for (auto& e : buyEntries) buyLabels.push_back(e.label);
    }

    void rebuildCurrentTab() {
        if (tabSelected == 0) {
            rebuildSell();
        }
        else {
            rebuildBuy();
        }
        selected = 0;
    }

    void checkLevelUpInternal() {
        int needed = 100 + player.level * 20;
        while (player.exp >= needed) {
            player.exp -= needed;
            player.level++;
            statusMessage = "Level Up! Current level: " + to_string(player.level);
            needed = 100 + player.level * 20;
        }
    }

    bool validSellSelection() {
        if (selected < 0 || selected >= (int)sellNames.size()) return false;
        if (sellNames[selected].empty()) return false;
        return sellPrices[selected] > 0 && sellQty[selected] > 0;
    }

public:
    // 构造函数新增两个蓝图 bool 引用
    TradeUI(PlayerData& p, ScreenInteractive& s,
            bool& genBP, bool& wireBP)
        : player(p), screen(s), genBPUnlocked(genBP), wireBPUnlocked(wireBP) {
        setupUI();
    }

    void setupUI() {
        rebuildCurrentTab();

        // ===== 共享按钮：CLOSE =====
        buttonClose = Button("CLOSE", [&] {
            running = false;
            screen.ExitLoopClosure()();
        });

        // ===== SELL 按钮 =====
        buttonSellOne = Button("SELL ONE", [&] {
            if (tabSelected != 0) return;
            if (!validSellSelection()) { statusMessage = "Cannot sell this."; return; }
            string name = sellNames[selected];
            if (player.removeItem(name, 1)) {
                int total = sellPrices[selected];
                player.coins += total;
                player.exp += 5;
                statusMessage = "Sold " + name + " x1, +" + to_string(total) + "c";
                rebuildSell();
                checkLevelUpInternal();
            }
        });
        buttonSellAll = Button("SELL ALL", [&] {
            if (tabSelected != 0) return;
            if (!validSellSelection()) { statusMessage = "Cannot sell this."; return; }
            string name = sellNames[selected];
            int qty = sellQty[selected];
            if (player.removeItem(name, qty)) {
                int total = sellPrices[selected] * qty;
                player.coins += total;
                player.exp += qty * 5;
                statusMessage = "Sold " + name + " x" + to_string(qty) + ", +" + to_string(total) + "c";
                rebuildSell();
                checkLevelUpInternal();
            }
        });
        buttonSellEverything = Button("SELL EVERYTHING", [&] {
            if (tabSelected != 0) return;
            vector<pair<string, int>> toSell;
            for (auto& item : player.inventory) {
                if (item.quantity > 0 && itemPrice(item.name) > 0) {
                    toSell.push_back({ item.name, item.quantity });
                }
            }
            int totalCoins = 0, totalItems = 0, totalExp = 0;
            for (auto& kv : toSell) {
                if (player.removeItem(kv.first, kv.second)) {
                    totalCoins += itemPrice(kv.first) * kv.second;
                    totalExp += kv.second * 5;
                    totalItems += kv.second;
                }
            }
            player.coins += totalCoins;
            player.exp += totalExp;
            statusMessage = totalItems > 0
                ? "Sold " + to_string(totalItems) + " items, +" + to_string(totalCoins) + "c"
                : "Nothing sellable.";
            rebuildSell();
            checkLevelUpInternal();
        });

        // ===== BUY 按钮 =====
        buttonBuyOne = Button("BUY 1", [&] {
            if (tabSelected != 1) return;
            rebuildBuy();  // 刷新蓝图 OWNED 状态，防止 label 过期
            if (selected < 0 || selected >= (int)buyEntries.size()) return;
            BuyEntry& e = buyEntries[selected];
            if (player.coins < e.buyPrice) {
                statusMessage = "Not enough coins.";
                return;
            }
            if (e.isBlueprint) {
                bool& flag = e.genBlueprint ? genBPUnlocked : wireBPUnlocked;
                if (flag) {
                    statusMessage = "You already own this blueprint.";
                    return;
                }
                player.coins -= e.buyPrice;
                flag = true;
                player.exp += 15;
                statusMessage = e.genBlueprint ? "Unlocked: Generator Blueprint!" : "Unlocked: Wire Blueprint!";
                checkLevelUpInternal();
            }
            else {
                if (e.itemName.empty()) return;
                // 原材料/零件：直接 addItem (type = "material", 买入价 ≈ itemPrice*3，
                // 这里 itemPrice 本身是原价，我们用 buyPrice/3 作为记录 basePrice，避免零除）
                int basePrice = max(1, e.buyPrice / 3);
                player.addItem(e.itemName, 1, "material", basePrice);
                player.coins -= e.buyPrice;
                player.exp += 2;
                statusMessage = "Bought " + e.itemName + " x1, -" + to_string(e.buyPrice) + "c";
                checkLevelUpInternal();
            }
        });

        // ===== Tab + Menu Radiobox =====
        tabLabels = { "  SELL  ","  BUY  " };
        tabBar = Radiobox(&tabLabels, &tabSelected);
        // 任何键盘事件后都检查 tabSelected 是否变化：变化就 rebuild（让 BUY 的 [OWNED]
        // 标签和 selected 索引立即同步，不依赖按回车）。
        tabBar |= CatchEvent([&](Event e) {
            if (tabSelected != lastTab) {
                lastTab = tabSelected;
                rebuildCurrentTab();
            }
            return false;
        });

        // 菜单一共有两个：SELL 和 BUY。用 Maybe 门控，让非当前 tab 的 Menu 失活，
        // 这样 FTXUI 容器焦点才会落到当前 tab 的菜单上（之前 BUY 选不中的根因）。
        Component menuSell = Menu(&sellEntries, &selected);
        // BUY 菜单：每次渲染前重建 buyLabels（类成员，避免悬空引用）
        // 注意：Renderer 的 lambda 必须按值捕获子组件 menuBuyChild，
        // 不能用 [&] —— 否则 menuBuy 重新赋值后引用会指向 Renderer 自身，调用 Render 时递归。
        genBuyLabels();
        Component menuBuy = Menu(&buyLabels, &selected);
        Component menuBuyChild = menuBuy;
        menuBuy = Renderer(menuBuyChild, [this, menuBuyChild] {
            genBuyLabels();
            if (selected >= (int)buyLabels.size()) selected = 0;
            return menuBuyChild->Render();
        });
        auto showSellTab = [this] { return tabSelected == 0; };
        auto showBuyTab  = [this] { return tabSelected == 1; };
        menu = Container::Vertical({
            Maybe(menuSell, showSellTab),
            Maybe(menuBuy,  showBuyTab),
        });

        // ===== 详情 renderer =====
        detailBox = Renderer([&] {
            if (tabSelected == 0) {
                // SELL
                if (selected < 0 || selected >= (int)sellNames.size() || sellNames[selected].empty()) {
                    return vbox({ text("  No item selected.") }) | border;
                }
                string name = sellNames[selected];
                int qty = sellQty[selected];
                int price = sellPrices[selected];
                int value = price * qty;
                return vbox({
                    text("  Item:   " + name) | bold,
                    text("  Stock:  x" + to_string(qty)),
                    text("  Price:  " + to_string(price) + "c each"),
                    text("  Total:  " + to_string(value) + "c"),
                    separator(),
                    price > 0 ? text("  Sellable.") | color(Color::Green)
                              : text("  Not sellable here.") | color(Color::Red),
                }) | border | size(WIDTH, GREATER_THAN, 36);
            }
            else {
                // BUY
                rebuildBuy();
                if (selected < 0 || selected >= (int)buyEntries.size()) {
                    return vbox({ text("  No item selected.") }) | border;
                }
                BuyEntry& e = buyEntries[selected];
                if (e.isBlueprint) {
                    bool owned = e.genBlueprint ? genBPUnlocked : wireBPUnlocked;
                    return vbox({
                        text("  " + string(e.genBlueprint ? "GENERATOR" : "WIRE") + " BLUEPRINT") | bold | color(Color::Yellow),
                        separator(),
                        text("  Cost:   " + to_string(e.buyPrice) + "c"),
                        text("  Owned:  " + string(owned ? "YES [OWNED]" : "NO")),
                        separator(),
                        owned ? text("  Already unlocked.") | color(Color::Cyan)
                              : text("  One-time unlock blueprint."),
                    }) | border | size(WIDTH, GREATER_THAN, 36);
                }
                return vbox({
                    text("  Item:   " + e.itemName) | bold,
                    text("  Cost:   " + to_string(e.buyPrice) + "c each"),
                    text("  Coins:  " + to_string(player.coins) + "c"),
                    separator(),
                    text("  Raw material / machine part."),
                }) | border | size(WIDTH, GREATER_THAN, 36);
            }
        });

        // ===== 布局：顶部 Tab，左侧菜单，右侧详情+按钮 =====
        // tabBar 必须挂进 main 容器，否则键盘焦点进不去（之前 BUY 选不了的根因之二）。
        // menuSell / menuBuy / 各按钮都用 Maybe 门控：非当前 tab 的组件失活，
        // 从焦点链移除。否则 BUY tab 时 SELL 按钮仍是活跃焦点目标但不可见，
        // 用户按 Enter 会命中幽灵按钮（内部 tabSelected 守卫直接 return → "没效果"）。
        auto main = Container::Vertical({
            tabBar,
            Maybe(menuSell,           showSellTab),
            Maybe(menuBuy,            showBuyTab),
            Maybe(buttonSellOne,      showSellTab),
            Maybe(buttonSellAll,      showSellTab),
            Maybe(buttonSellEverything, showSellTab),
            Maybe(buttonBuyOne,       showBuyTab),
            buttonClose,
        });

        mainContainer = Renderer(main, [&, menuSell, menuBuy] {
            auto topBar = hbox({
                text(" TAB: ") | bold,
                tabBar->Render(),
            }) | border;
            return vbox({
                text("            $ TRADING MARKET $") | bold | color(Color::Yellow),
                separator(),
                hbox({ text(" Coins: ") | bold, text(to_string(player.coins)) | color(Color::Yellow) }),
                separator(),
                topBar,
                separator(),
                hbox({
                    vbox({
                        text(tabSelected == 0 ? " YOUR BACKPACK" : " BUY CATALOG") | bold | color(Color::Cyan),
                        separator(),
                        (tabSelected == 0 ? menuSell->Render() : menuBuy->Render()) | flex,
                    }) | flex,
                    vbox({
                        detailBox->Render(),
                        separator(),
                        tabSelected == 0 ? vbox({
                            buttonSellOne->Render() | flex,
                            buttonSellAll->Render() | flex,
                            buttonSellEverything->Render() | flex,
                            separator(),
                            buttonClose->Render() | flex,
                        }) : vbox({
                            buttonBuyOne->Render() | flex,
                            separator(),
                            buttonClose->Render() | flex,
                        }),
                    }),
                }) | flex,
                separator(),
                text(statusMessage) | color(Color::Green),
                text(tabSelected == 0
                    ? string("Tip: UP/DOWN on TAB to switch SELL/BUY; LEFT item pick; SELL buttons to sell.")
                    : string("Tip: UP/DOWN on TAB to switch SELL/BUY; LEFT item pick; BUY 1 to purchase (blueprints one-shot)."))
                    | color(Color::GrayDark),
            }) | border | size(WIDTH, GREATER_THAN, 84);
        });

        // ESC 直接关闭交易界面（和 BuildUI 一致）
        mainContainer = mainContainer | CatchEvent([&](Event e) {
            if (e == Event::Escape) {
                running = false;
                screen.ExitLoopClosure()();
                return true;
            }
            return false;
        });
    }

    Component getComponent() { return mainContainer; }
    bool isRunning() { return running; }
};

// ======================== 游戏主程序 ========================
class ChemicalWorldGame {
private:
    PlayerData player;
    GameMap gameMap;
    BlastFurnace furnace;
    Lathe lathe;
    Area currentArea = Area::Home;
    bool running = true;
    int activeSaveSlot = 1;
    string message = "Welcome to Chemical World! WASD to move, E to interact, F to open furnace";
    bool tutorialActive = true;
    int tutorialStep = 0;

    // ============== 电力与建造 ==============
    static const int EU_MAX = 10000;
    int globalEU = 0;
    bool gen_blueprint_unlocked = false;
    bool wire_blueprint_unlocked = false;
    bool crusher_blueprint_unlocked = false;
    bool washer_blueprint_unlocked = false;
    bool centrifuge_blueprint_unlocked = false;
    vector<MachineMeta> machineMeta;
    map<pair<int, int>, PowerGenerator> generators;
    map<pair<int, int>, Crusher> crushers;
    map<pair<int, int>, OreWasher> washers;
    map<pair<int, int>, Centrifuge> centrifuges;
    set<pair<int, int>> poweredMachines;

    bool powerDraw(int eu, pair<int, int> machineXY) {
        if (currentArea != Area::Home) return false;
        if (!poweredMachines.count(machineXY)) return false;
        if (globalEU < eu) return false;
        globalEU -= eu;
        return true;
    }

    struct TutorialStep {
        string title;
        string description;
        string action;
        bool completed = false;
    };


    vector<TutorialStep> tutorials = {
        {"Move & Explore", "Use WASD keys to move on the map\nTry moving to nearby water sources or mining spots", "move", false},
        {"Collect Resources", "Move into a hidden deposit and strike it 3 times with WASD\nFind hematite, magnetite, or coal first", "collect", false},
        {"Check Backpack", "Press C to open your backpack\nView your ores, fuels, steel parts and blueprints", "craft", false},
        {"Trading System", "Press T to open the trading menu\nSell your steel for coins", "trade", false},
        {"Blast Furnace", "Press F to open the furnace interface\nSmelt metals using ores and coal", "furnace", false}
    };

    string savePath(int slot) const {
        return slot == 0 ? "chemical_world_autosave.sav" :
            "chemical_world_slot" + to_string(slot) + ".sav";
    }

    bool saveGame(int slot) {
        ofstream output(savePath(slot));
        if (!output) return false;
        output << "CHEMICAL_WORLD_SAVE 4\n";
        output << player.name << '\n' << player.level << ' ' << player.coins << ' '
            << player.exp << ' ' << player.x << ' ' << player.y << '\n';
        output << static_cast<int>(currentArea) << '\n';
        output << player.inventory.size() << '\n';
        for (const auto& item : player.inventory)
            output << item.name << ' ' << item.quantity << ' ' << item.category << ' ' << item.value << '\n';
        output << tutorials.size() << '\n';
        for (const auto& tutorial : tutorials) output << (tutorial.completed ? 1 : 0) << ' ';
        output << '\n';
        gameMap.save(output);

        // === v4 追加段（5 个蓝图状态：gen/wire 沿用 v3 前缀，crusher/washer/centrifuge 为 v4 新增）===
        int tmp = 0;
        tmp = gen_blueprint_unlocked ? 1 : 0;          output << tmp << ' ';
        tmp = wire_blueprint_unlocked ? 1 : 0;         output << tmp << ' ';
        tmp = crusher_blueprint_unlocked ? 1 : 0;      output << tmp << ' ';
        tmp = washer_blueprint_unlocked ? 1 : 0;       output << tmp << ' ';
        tmp = centrifuge_blueprint_unlocked ? 1 : 0;   output << tmp << '\n';
        output << globalEU << '\n';

        // 把 generators 燃烧数据同步进 machineMeta 再统一保存
        for (auto& m : machineMeta) {
            if (m.type == 'G') {
                auto it = generators.find({ m.x, m.y });
                if (it != generators.end()) {
                    m.remainingBurnEU = it->second.burnEU;
                    m.loadedCoal = it->second.loadedCoal;
                    m.active = it->second.active;
                }
            }
        }
        output << machineMeta.size() << '\n';
        for (const auto& m : machineMeta) {
            tmp = m.active ? 1 : 0;
            output << m.x << ' ' << m.y << ' ' << m.type << ' '
                << m.remainingBurnEU << ' ' << m.loadedCoal << ' ' << tmp << '\n';
        }
        return output.good();
    }

    bool loadGame(int slot) {
        ifstream input(savePath(slot));
        string header;
        int version = 0;
        if (!input || !(input >> header >> version) || header != "CHEMICAL_WORLD_SAVE" ||
            (version != 2 && version != 3 && version != 4)) return false;
        if (!(input >> player.name >> player.level >> player.coins >> player.exp >> player.x >> player.y)) return false;
        int savedArea = 0;
        if (!(input >> savedArea) || savedArea < 0 || savedArea > 2) return false;
        currentArea = static_cast<Area>(savedArea);
        size_t itemCount = 0;
        if (!(input >> itemCount)) return false;
        player.inventory.clear();
        for (size_t i = 0; i < itemCount; ++i) {
            InventoryItem item;
            if (!(input >> item.name >> item.quantity >> item.category >> item.value)) return false;
            player.inventory.push_back(item);
        }
        size_t tutorialCount = 0;
        if (!(input >> tutorialCount) || tutorialCount != tutorials.size()) return false;
        for (auto& tutorial : tutorials) {
            int completed = 0;
            if (!(input >> completed)) return false;
            tutorial.completed = completed != 0;
        }
        tutorialActive = false;
        for (const auto& tutorial : tutorials) if (!tutorial.completed) tutorialActive = true;
        if (!gameMap.load(input)) return false;

        // === v2/v3/v4 新字段 ===
        gen_blueprint_unlocked = false;
        wire_blueprint_unlocked = false;
        crusher_blueprint_unlocked = false;
        washer_blueprint_unlocked = false;
        centrifuge_blueprint_unlocked = false;
        globalEU = 0;
        machineMeta.clear();
        generators.clear();
        crushers.clear();
        washers.clear();
        centrifuges.clear();
        poweredMachines.clear();
        furnace = BlastFurnace{};
        lathe = Lathe{};

        // 先从当前地图扫描默认的 F/L 机器 meta（v2 没有 meta 段时这就是唯一来源）
        for (int y = 0; y < gameMap.getHeight(); ++y) {
            for (int x = 0; x < gameMap.getWidth(); ++x) {
                char d = gameMap.getTileDisplay(x, y);
                if (d == 'F' || d == 'L') {
                    machineMeta.push_back({ x, y, d, 0, 0, false });
                }
            }
        }
        if (version == 3 || version == 4) {
            int tmp = 0;
            if (!(input >> tmp)) return false; gen_blueprint_unlocked = !!tmp;
            if (!(input >> tmp)) return false; wire_blueprint_unlocked = !!tmp;
            // v4 追加 3 个新机器蓝图；v3 存档无此字段，保持默认 false
            if (version >= 4) {
                if (!(input >> tmp)) return false; crusher_blueprint_unlocked = !!tmp;
                if (!(input >> tmp)) return false; washer_blueprint_unlocked = !!tmp;
                if (!(input >> tmp)) return false; centrifuge_blueprint_unlocked = !!tmp;
            }
            if (!(input >> globalEU)) return false;
            size_t metaCount = 0;
            if (!(input >> metaCount)) return false;
            machineMeta.clear();  // v3/v4 的 meta 覆盖上面的扫描默认
            for (size_t i = 0; i < metaCount; ++i) {
                MachineMeta m;
                if (!(input >> m.x >> m.y >> m.type >> m.remainingBurnEU >> m.loadedCoal >> tmp)) return false;
                m.active = !!tmp;
                machineMeta.push_back(m);
                if (m.type == 'G') {
                    PowerGenerator g;
                    g.burnEU = m.remainingBurnEU;
                    g.loadedCoal = m.loadedCoal;
                    g.active = m.active;
                    generators[{m.x, m.y}] = g;
                }
                // X/W/R 机器实例在打开各自 UI 时按 find-or-insert 重建，
                // 这里只需保留 machineMeta 条目（用于 E 键查找与地图渲染）。
            }
        }
        return true;
    }

    void newGame() {
        player = PlayerData{};
        gameMap = GameMap{};
        furnace = BlastFurnace{};
        lathe = Lathe{};
        currentArea = Area::Home;
        gameMap.generate(currentArea);
        player.x = 10;   // 玩家出生 (10,5)，避开车床 L 2×2 (8,5)-(9,6)
        player.y = 5;
        player.name = "Chemist";
        player.addItem("water", 5, "basic", 3);
        player.addItem("coal", 5, "fuel", 8);
        player.addItem("sand", 3, "material", 2);
        for (auto& tutorial : tutorials) tutorial.completed = false;
        tutorialActive = true;
        // 电力与建造默认状态
        globalEU = 0;
        gen_blueprint_unlocked = false;
        wire_blueprint_unlocked = false;
        crusher_blueprint_unlocked = false;
        washer_blueprint_unlocked = false;
        centrifuge_blueprint_unlocked = false;
        generators.clear();
        crushers.clear();
        washers.clear();
        centrifuges.clear();
        poweredMachines.clear();
        machineMeta.clear();
        machineMeta.push_back({ 5, 5, 'F', 0, 0, false });   // BlastFurnace 左上 (5,5)
        machineMeta.push_back({ 8, 5, 'L', 0, 0, false });   // Lathe 左上 (8,5)
        message = "Welcome to Chemical World!";
    }

    void introAnimation() {
        const vector<string> frames = {
            "      C H E M I C A L   W O R L D      ",
            "      [ C H E M I C A L   W O R L D ]  ",
            "      [ C H E M I C A L   W O R L D ]\n          Explore. Smelt. Build.       "
        };
        for (const auto& frame : frames) {
            cls();
            setcolor(COLOR_CYAN);
            cout << "\n\n\n" << frame << "\n";
            setcolor(COLOR_GREY);
            cout << "\n              INITIALIZING WORLD...\n";
            setcolor(COLOR_RESET);
            this_thread::sleep_for(chrono::milliseconds(350));
        }
        this_thread::sleep_for(chrono::milliseconds(450));
    }

    int selectSaveSlot() {
        vector<string> slots;
        for (int slot = 0; slot <= 3; ++slot) {
            ifstream input(savePath(slot));
            string label = slot == 0 ? "AUTO SAVE" : "MANUAL SLOT " + to_string(slot);
            if (!input) label += "  [EMPTY]";
            else {
                // 读取存档信息
                string header, name;
                int version, level, coins, exp, x, y;
                if (input >> header >> version >> name >> level >> coins >> exp >> x >> y) {
                    label += "  [Lv." + to_string(level) + " " + name + "]";
                }
            }
            slots.push_back(label);
        }

        int selected = 0;
        int result = -3;
        auto screen = ScreenInteractive::Fullscreen();

        // 创建标题艺术字
        auto titleArt = text("  .----------------.  .----------------.  .----------------.  .----------------. ") | color(Color::Cyan);
        auto titleArt2 = text("  | .--------------. || .--------------. || .--------------. || .--------------. |") | color(Color::Cyan);
        auto titleArt3 = text("  | |    _____     | || |   ______    | || |  ____       | || |   _____      | |") | color(Color::Cyan);
        auto titleArt4 = text("  | |   / ___ `.  | || |  |_   _ \\   | || | |_  _|      | || |  |_   _|     | |") | color(Color::Cyan);
        auto titleArt5 = text("  | |  |_/___) | | || |    | |_) |  | || |   \\ \\       | || |    | |       | |") | color(Color::Cyan);
        auto titleArt6 = text("  | |   .'____.'  | || |    |  __'.  | || |    \\ \\      | || |    | |   _   | |") | color(Color::Cyan);
        auto titleArt7 = text("  | |  / /____    | || |   _| |__) | | || |     \\ \\     | || |   _| |__/ |  | |") | color(Color::Cyan);
        auto titleArt8 = text("  | |  |_______|  | || |  |_______/  | || |      \\ \\    | || |  |________|  | |") | color(Color::Cyan);
        auto titleArt9 = text("  | |              | || |              | || |       \\ \\   | || |              | |") | color(Color::Cyan);
        auto titleArt10 = text("  | '--------------' || '--------------' || '--------------' || '--------------' |") | color(Color::Cyan);
        auto titleArt11 = text("  '----------------'  '----------------'  '----------------'  '----------------' ") | color(Color::Cyan);

        auto titleBlock = vbox({
            titleArt,
            titleArt2,
            titleArt3,
            titleArt4,
            titleArt5,
            titleArt6,
            titleArt7,
            titleArt8,
            titleArt9,
            titleArt10,
            titleArt11,
            });

        auto subtitle = text("           ~ A World of Elements and Discovery ~") | bold | color(Color::Yellow);

        // 分隔线
        auto divider = separator() | color(Color::GrayDark);

        auto slotList = Radiobox(&slots, &selected);

        // 按钮
        auto load = Button("LOAD SELECTED", [&] { result = selected; screen.ExitLoopClosure()(); });
        auto fresh = Button("NEW GAME", [&] { result = -2; screen.ExitLoopClosure()(); });
        auto quit = Button("QUIT", [&] { result = -1; screen.ExitLoopClosure()(); });

        auto buttons = Container::Horizontal({ load, fresh, quit });

        // 脚注信息
        auto footer = text("  [Arrow keys to navigate]  [Enter to select]  [Tab between buttons]") | color(Color::GrayDark);

        auto layout = Container::Vertical({ slotList, buttons });

        auto view = Renderer(layout, [&] {
            Elements infoLines;
            infoLines.push_back(text("  +------------------------------------------------------------------+") | color(Color::GrayDark));
            infoLines.push_back(text("  |  Use UP/DOWN to choose a save slot                              |") | color(Color::GrayDark));
            infoLines.push_back(text("  |  Press ENTER to load the selected save                          |") | color(Color::GrayDark));
            infoLines.push_back(text("  |  [NEW GAME] starts a fresh adventure                            |") | color(Color::GrayDark));
            infoLines.push_back(text("  |  [QUIT] exits the game                                         |") | color(Color::GrayDark));
            infoLines.push_back(text("  +------------------------------------------------------------------+") | color(Color::GrayDark));

            return vbox({
                filler(),
                titleBlock | center,
                text("") | size(HEIGHT, EQUAL, 1),
                subtitle | center,
                text("") | size(HEIGHT, EQUAL, 1),
                divider,
                text("") | size(HEIGHT, EQUAL, 1),
                text("  SELECT A WORLD") | bold | color(Color::Yellow) | center,
                text("") | size(HEIGHT, EQUAL, 1),
                slotList->Render() | border | size(WIDTH, GREATER_THAN, 55) | center,
                text("") | size(HEIGHT, EQUAL, 1),
                buttons->Render() | center | flex,
                text("") | size(HEIGHT, EQUAL, 1),
                divider,
                vbox(infoLines) | center,
                text("") | size(HEIGHT, EQUAL, 1),
                footer | center,
                filler(),
                }) | border | size(WIDTH, GREATER_THAN, 78) | center;
            });

        screen.Loop(view);
        return result;
    }

    string getTutorialText() {
        if (!tutorialActive) return "";
        for (auto& step : tutorials) {
            if (!step.completed) {
                return "@ " + step.title + "\n   " + step.description;
            }
        }
        tutorialActive = false;
        return "^_^ Tutorial complete! Explore freely!";
    }

    void checkTutorialProgress(const string& action) {
        if (!tutorialActive) return;
        for (auto& step : tutorials) {
            if (!step.completed && step.action == action) {
                step.completed = true;
                message = "! Tutorial complete: " + step.title + "! Continue to the next challenge!";
                return;
            }
        }
    }

    // 背包界面（FTXUI 图形化）：C 键打开，左侧分类物品列表 + 右侧详情
    void openBackpack() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();

        // 分类顺序（人性化命名 + 颜色）
        struct CatInfo { string key; string label; Color color; };
        vector<CatInfo> cats = {
            { "ore",      "ORES / MINERALS",   Color::Yellow },
            { "fuel",     "FUELS",              Color::Red },
            { "material", "MATERIALS",          Color::Cyan },
            { "product",  "STEEL PARTS",        Color::Green },
            { "blueprint","BLUEPRINTS",         Color::Magenta },
            { "misc",     "MISC",               Color::GrayDark },
        };

        int selected = 0;
        Component menu;
        vector<string> labels;  // 每个条目 "name xqty  [category]"

        // 分类汇总当前可显示条目（数量 > 0）
        auto rebuild = [&]() {
            labels.clear();
            for (auto& c : cats) {
                bool catHasItem = false;
                for (auto& it : player.inventory) {
                    if (it.quantity > 0 && it.category == c.key) {
                        labels.push_back("  " + it.name + "  x" + to_string(it.quantity) + "  [" + c.key + "]");
                        catHasItem = true;
                    }
                }
                if (!catHasItem) {
                    labels.push_back("  (none in " + c.key + ")");
                }
            }
            if (labels.empty()) {
                labels.push_back("  (backpack empty)");
            }
            if (selected >= (int)labels.size()) selected = 0;
        };
        rebuild();

        menu = Menu(&labels, &selected);

        // 关闭按钮
        Component buttonClose = Button("CLOSE", [&] { screen.ExitLoopClosure()(); });
        auto buttons = Container::Vertical({ buttonClose });

        // 主容器：menu + 按钮（焦点链）
        auto layout = Container::Vertical({ menu, buttons });

        Component view = Renderer(layout, [&] {
            // 找到选中的实际 InventoryItem（跳过分隔行 "(none...)"）
            InventoryItem* sel = nullptr;
            int idx = 0;
            for (auto& c : cats) {
                bool catHasItem = false;
                for (auto& it : player.inventory) {
                    if (it.quantity > 0 && it.category == c.key) {
                        if (idx == selected) { sel = &it; break; }
                        idx++;
                        catHasItem = true;
                    }
                }
                if (!catHasItem) idx++;  // 跳过 "(none)" 占位
                if (sel) break;
            }

            // 左侧列表：用分类标题分组渲染（不是所有 labels 直接 dump）
            Elements leftRows;
            leftRows.push_back(text(" BACKPACK") | bold | color(Color::Yellow));
            leftRows.push_back(separator());
            int row = 0;
            for (auto& c : cats) {
                leftRows.push_back(text("[" + c.label + "]") | bold | color(c.color));
                bool any = false;
                for (auto& it : player.inventory) {
                    if (it.quantity > 0 && it.category == c.key) {
                        string prefix = (row == selected) ? " > " : "   ";
                        Color rowCol = (row == selected) ? Color::Yellow : Color::White;
                        leftRows.push_back(hbox({
                            text(prefix) | color(rowCol),
                            text(it.name + "  x" + to_string(it.quantity)) | color(rowCol),
                        }));
                        row++;
                        any = true;
                    }
                }
                if (!any) {
                    leftRows.push_back(text("   (empty)") | color(Color::GrayDark));
                    row++;
                }
            }

            // 右侧详情面板
            Component detail;
            if (sel) {
                string catLabel = sel->category;
                for (auto& c : cats) if (c.key == sel->category) { catLabel = c.label; break; }
                detail = Renderer([&, sel, catLabel] {
                    return vbox({
                        text(" ITEM DETAILS") | bold | color(Color::Cyan),
                        separator(),
                        text("  Name:     " + sel->name) | bold,
                        text("  Category: " + catLabel),
                        text("  Quantity: x" + to_string(sel->quantity)) | color(Color::Yellow),
                        text("  Value:    " + to_string(sel->value) + " coins each"),
                        text("  Total:    " + to_string(sel->value * sel->quantity) + " coins"),
                        separator(),
                        text("  Tip: Trade at T (SELL) to convert items to coins.") | color(Color::GrayDark),
                        text("       Feed coal to generator (E) or load furnace (E) to use.") | color(Color::GrayDark),
                    });
                });
            }
            else {
                detail = Renderer([&, sel] {
                    return vbox({
                        text(" ITEM DETAILS") | bold | color(Color::Cyan),
                        separator(),
                        text("  (no item selected)") | color(Color::GrayDark),
                        text("  Use UP/DOWN on the list, then read details here.") | color(Color::GrayDark),
                    });
                });
            }

            return vbox({
                text("            # BACKPACK [C] #") | bold | color(Color::Yellow),
                separator(),
                hbox({
                    vbox(leftRows) | flex,
                    separator(),
                    detail->Render() | flex,
                }),
                separator(),
                text("Total items: " + to_string(player.inventory.size()) +
                     "   Coins: " + to_string(player.coins) +
                     "   Level: " + to_string(player.level) +
                     "   EU: " + to_string(globalEU)) | color(Color::Green),
                separator(),
                buttons->Render() | center,
                text("Tip: UP/DOWN select item, TAB to CLOSE button, ESC to exit.") | color(Color::GrayDark),
            }) | border | size(WIDTH, GREATER_THAN, 80);
        });

        // ESC 关闭
        view = view | CatchEvent([&](Event e) {
            if (e == Event::Escape) {
                screen.ExitLoopClosure()();
                return true;
            }
            return false;
        });

        screen.Loop(view);

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        checkTutorialProgress("craft");
        message = "& Backpack closed.";
        cls();
    }

    // 交易系统（FTXUI 图形化界面）
    void trade() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();
        TradeUI tradeUI(player, screen, gen_blueprint_unlocked, wire_blueprint_unlocked);
        screen.Loop(tradeUI.getComponent());

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        checkTutorialProgress("trade");
        message = "& Trading complete!";
        cls();
    }

    int getItemPrice(const string& item) {
        return itemPrice(item);
    }

    void checkLevelUp() {
        int needed = 100 + player.level * 20;
        while (player.exp >= needed) {
            player.exp -= needed;
            player.level++;
            message = "QwQ Level Up! Current level: " + to_string(player.level);
            needed = 100 + player.level * 20;
        }
    }

    void travelTo(Area destination) {
        const vector<string> frames = {
            "          ________________________________",
            "     ____/  _  _  _  _  _  _  _  _  _  _  \\____",
            " ___/   _| |_| |_| |_| |_| |_| |_| |_| |_| |_   \\___",
            "|     _/        .        .        .        \\_     |",
            "|____/_____.________.________.________.____\\_____|",
            "             O---O                                  O---O",
            "             THE ROAD RUNS ON..."
        };
        for (const auto& frame : frames) {
            cls();
            setcolor(COLOR_YELLOW);
            cout << "\n\n" << frame << "\n";
            setcolor(COLOR_RESET);
            this_thread::sleep_for(chrono::milliseconds(220));
        }
        currentArea = destination;
        gameMap.generate(currentArea);
        if (destination == Area::Wasteland) {
            player.x = 4; player.y = 2;
            message = "Welcome to the wasteland. Find a cave entrance marked O.";
        }
        else {
            player.x = 10; player.y = 5;
            message = "Back home. Machines can only be used here.";
        }
    }

    // 打开高炉UI（使用FTXUI）
    void openFurnaceUI() {
        // 隐藏光标
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        // 创建FTXUI界面
        auto screen = ScreenInteractive::Fullscreen();
        FurnaceUI furnaceUI(furnace, player, screen);

        thread furnaceTicker([&]() {
            while (furnaceUI.isRunning()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                globalTick100ms();
                if (furnaceUI.isRunning()) {
                    screen.PostEvent(Event::Custom);
                }
            }
            });

        // 运行界面
        screen.Loop(furnaceUI.getComponent());

        if (furnaceTicker.joinable()) {
            furnaceTicker.join();
        }

        // 恢复光标
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        checkTutorialProgress("furnace");
        message = "& Furnace operation complete!";
        cls();
    }

    // 打开车床 UI（镜像 openFurnaceUI：隐光标→Fullscreen→ticker 线程→Loop→join→恢复光标）
    void openLatheUI() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();
        LatheUI latheUI(lathe, player, screen);

        thread latheTicker([&]() {
            while (latheUI.isRunning()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                globalTick100ms();
                if (latheUI.isRunning()) {
                    screen.PostEvent(Event::Custom);
                }
            }
            });

        screen.Loop(latheUI.getComponent());

        if (latheTicker.joinable()) {
            latheTicker.join();
        }

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        // 不调 checkTutorialProgress：避免新增教程步破坏旧存档（tutorialCount 守卫）
        message = "& Lathe operation complete!";
        cls();
    }

    void renderMap() {
        setcolor(COLOR_CYAN);
        cout << "==================================================\n";
        string areaName = currentArea == Area::Home ? "HOME" :
            (currentArea == Area::Wasteland ? "WASTELAND" : "CAVE");
        cout << "|                  " << areaName << " MAP                   |\n";
        cout << "==================================================\n\n";
        setcolor(COLOR_RESET);

        int width = gameMap.getWidth();
        int height = gameMap.getHeight();

        int viewX = max(0, player.x - 12);
        int viewY = max(0, player.y - 8);
        int viewWidth = min(width - viewX, 25);
        int viewHeight = min(height - viewY, 17);

        for (int y = viewY; y < viewY + viewHeight; y++) {
            cout << "  ";
            for (int x = viewX; x < viewX + viewWidth; x++) {
                if (x == player.x && y == player.y) {
                    setcolor(COLOR_YELLOW);
                    cout << 'P';
                    setcolor(COLOR_RESET);
                }
                else {
                    Tile& tile = gameMap.getTile(x, y);
                    setcolor((int)tile.color);
                    bool animal = currentArea == Area::Home && tile.display == '.' &&
                        ((x * 17 + y * 31 + static_cast<int>(time(nullptr)) * 3) % 97 == 0);
                    cout << (animal ? 'a' : tile.display);
                    setcolor(COLOR_RESET);
                }
                cout << ' ';
            }
            cout << '\n';
        }

        // ===== 图例（按区域显示实际出现的符号；矿石仅出现在 Wasteland/Cave）=====
        setcolor(COLOR_RESET);
        cout << "\n  Legend:  ";
        setcolor(COLOR_YELLOW); cout << "P=You  ";
        setcolor(COLOR_GREEN);  cout << ".=Grass  ";
        if (currentArea == Area::Home) {
            setcolor(COLOR_DARK_GREEN); cout << "t=Tree  v=Grass_tuft  ";
            setcolor(COLOR_BLUE);     cout << "~=Water  ";
            setcolor(COLOR_YELLOW);   cout << "*=Flower  a=Animal  |  ";
            setcolor(COLOR_RED);      cout << "F=Furnace  ";
            setcolor(COLOR_PURPLE);   cout << "L=Lathe  ";
            setcolor(COLOR_DARK_RED); cout << "G=Generator  ";
            setcolor(COLOR_YELLOW);   cout << "X=Crusher  ";
            setcolor(COLOR_CYAN);     cout << "W=Washer  ";
            setcolor(COLOR_PURPLE);   cout << "R=Centrifuge  ";
            setcolor(COLOR_BLUE);     cout << "+=Wire  ";
            setcolor(COLOR_YELLOW);   cout << "C=Car(travel)";
        } else {
            if (currentArea == Area::Wasteland) {
                setcolor(COLOR_DARK_YELLOW); cout << "=Dry_ground  ";
                setcolor(COLOR_LIGHT_WHITE); cout << "~=Salt_Flats  ";
                setcolor(COLOR_RED);     cout << "^=Red_Dune  ";
                setcolor(COLOR_YELLOW);  cout << "C=Car  ";
                setcolor(COLOR_CYAN);    cout << "O=Cave_entrance  |  ";
            } else {
                setcolor(COLOR_GREY);  cout << "=Cave_floor  ";
                setcolor(COLOR_CYAN);  cout << "O=Cave_exit  |  ";
            }
            setcolor(COLOR_GREY);     cout << "==Rock  ";
            setcolor(COLOR_DARK_RED); cout << "Ore: H=Hematite M=Magnetite B=Bauxite T=Cassiterite P=Malachite U=Chalcopyrite G=Gold S=Silver C=Coal";
            if (currentArea == Area::Wasteland) cout << " (C also=Car)";
        }
        cout << "\n";
        setcolor(COLOR_RESET);
    }

    void showStatus() {
        string areaName = currentArea == Area::Home ? "HOME" :
            (currentArea == Area::Wasteland ? "WASTELAND" : "CAVE");
        setcolor(COLOR_YELLOW);
        cout << "\n==================================================\n";
        cout << "|  " << player.name << " | Lv." << player.level;
        cout << " | Coins: " << player.coins;
        cout << " | Exp: " << player.exp;
        cout << string(30 - (player.name.length() + to_string(player.level).length() +
            to_string(player.coins).length() + to_string(player.exp).length()), ' ');
        cout << "|\n";
        cout << "|  Area: " << areaName << "  Position: (" << player.x << ", " << player.y << ")";
        cout << string(48 - (to_string(player.x).length() + to_string(player.y).length()), ' ');
        cout << "|\n";
        cout << "==================================================\n";
        setcolor(COLOR_RESET);
    }

    void showInventory() {
        setcolor(COLOR_GREEN);
        cout << "$ Inventory: ";
        if (player.inventory.empty()) {
            cout << "(empty)";
        }
        else {
            for (auto& item : player.inventory) {
                if (item.quantity > 0) {
                    cout << item.name << " x" << item.quantity << " ";
                }
            }
        }
        cout << "\n";
        setcolor(COLOR_RESET);
    }

    void showMessage() {
        setcolor(COLOR_LIGHT_WHITE);
        cout << "\n# " << message << "\n";
        setcolor(COLOR_RESET);
        message = "";
    }

    void showTutorial() {
        if (!tutorialActive) return;
        setcolor(COLOR_CYAN);
        string text = getTutorialText();
        if (!text.empty()) {
            cout << "\n" << text << "\n";
        }
        setcolor(COLOR_RESET);
    }

    void showHelp() {
        setcolor(COLOR_CYAN);
        cout << "\n==================================================\n";
        cout << "|                   Controls                      |\n";
        cout << "==================================================\n";
        setcolor(COLOR_GREEN);
        cout << "  WASD  - Move\n";
        cout << "  E     - Interact nearby (mine/enter/use machine)\n";
        cout << "  C     - Open backpack\n";
        cout << "  T     - Trading market (buy blueprints/sell goods)\n";
        cout << "  B     - Build menu (home only): place generator/wire/machines/decor\n";
        cout << "  F     - Hint only; use E beside furnace at home\n";
        cout << "  H     - Show help\n";
        cout << "  P     - Save to active manual slot\n";
        cout << "  L     - Load active manual slot\n";
        cout << "  Q     - Quit game\n";
        setcolor(COLOR_RESET);
    }

    void processKey(int key) {
        int newX = player.x, newY = player.y;

        switch (key) {
        case 'w': case 'W': newY--; break;
        case 's': case 'S': newY++; break;
        case 'a': case 'A': newX--; break;
        case 'b': case 'B':
            if (currentArea == Area::Home) openBuildUI();
            else message = "Can only build at home.";
            return;
        case 'd': case 'D': newX++; break;
        case 'e': case 'E':
            if (currentArea == Area::Home && gameMap.isNear(5, 5, 'F', player.x, player.y)) {
                openFurnaceUI();
                return;
            }
            if (currentArea == Area::Home && gameMap.isNear(5, 8, 'L', player.x, player.y)) {
                openLatheUI();
                return;
            }
            if ((currentArea == Area::Home || currentArea == Area::Wasteland) &&
                gameMap.isNear(currentArea == Area::Home ? 12 : 2, currentArea == Area::Home ? 5 : 2,
                    'C', player.x, player.y)) {
                travelTo(currentArea == Area::Home ? Area::Wasteland : Area::Home);
                return;
            }
            if (currentArea == Area::Wasteland && gameMap.isNear(gameMap.getWidth() / 2, gameMap.getHeight() / 2, 'O', player.x, player.y)) {
                currentArea = Area::Cave;
                gameMap.generate(currentArea);
                player.x = 2;
                player.y = 2;
                message = "You entered a cave. Search the grey rock for exposed ore.";
                return;
            }
            if (currentArea == Area::Cave && gameMap.isNear(1, 1, 'O', player.x, player.y)) {
                currentArea = Area::Wasteland;
                gameMap.generate(currentArea);
                player.x = 4;
                player.y = 2;
                message = "You climbed out of the cave.";
                return;
            }
            // Generator interaction (isNear any installed G anchor)
            {
                pair<int, int> gpos;
                if (currentArea == Area::Home && findNearbyGenerator(player.x, player.y, gpos)) {
                    openGeneratorPanel(gpos.first, gpos.second);
                    return;
                }
            }
            // Crusher / Washer / Centrifuge interaction
            {
                pair<int, int> mpos;
                if (currentArea == Area::Home && findNearbyMachine('X', player.x, player.y, mpos)) {
                    openCrusherUI(mpos.first, mpos.second);
                    return;
                }
                if (currentArea == Area::Home && findNearbyMachine('W', player.x, player.y, mpos)) {
                    openWasherUI(mpos.first, mpos.second);
                    return;
                }
                if (currentArea == Area::Home && findNearbyMachine('R', player.x, player.y, mpos)) {
                    openCentrifugeUI(mpos.first, mpos.second);
                    return;
                }
            }
            for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
                if (abs(dx) + abs(dy) == 1 && gameMap.getTile(player.x + dx, player.y + dy).mineral != "") {
                    if (gameMap.mineAt(player, player.x + dx, player.y + dy)) {
                        message = "Mining " + gameMap.getTile(player.x + dx, player.y + dy).name + " with E";
                        checkTutorialProgress("collect");
                    }
                    return;
                }
            }
            message = "Nothing nearby to interact with.";
            return;
        case 'c': case 'C':
            openBackpack();
            return;
        case 't': case 'T':
            trade();
            return;
        case 'f': case 'F':
            message = "Find the furnace at home and press E beside it.";
            return;
        case 'h': case 'H':
            cls();
            showHelp();
            cout << "\nPress any key to continue...";
            _getch();
            return;
        case 'q': case 'Q':
            saveGame(0);
            running = false;
            return;
        case 'P':case 'p' :
            if (saveGame(activeSaveSlot)) message = "Game saved to Manual Slot " + to_string(activeSaveSlot) + ".";
            else message = "X Could not save the game.";
            return;
		case 'L':case 'l' :
            if (loadGame(activeSaveSlot)) message = "Game loaded from Manual Slot " + to_string(activeSaveSlot) + ".";
            else message = "X No valid save in Manual Slot " + to_string(activeSaveSlot) + ".";
            return;
        default:
            return;
        }

        if (gameMap.isPassable(newX, newY)) {
            player.x = newX;
            player.y = newY;
            checkTutorialProgress("move");
            message = "Moved to (" + to_string(player.x) + ", " + to_string(player.y) + ")";
        }
        else {
            message = "X Blocked by obstacle!";
        }
    }

    // ===== 电力与建造（任务 2 实现）=====

    // BFS 计算电网连通性 + 驱动所有发电机燃烧 EU 注入 globalEU；把连通集合缓存进 poweredMachines
    void tickPowerGrid() {
        poweredMachines.clear();
        if (currentArea != Area::Home) return;
        const int W = gameMap.getWidth();
        const int H = gameMap.getHeight();

        // 1) 初始化 BFS 队列：把所有发电机 'G' 主格（keys of generators）入队
        set<pair<int, int>> visited;
        vector<pair<int, int>> queue;
        for (auto& kv : generators) {
            int gx = kv.first.first, gy = kv.first.second;
            // 2×2 G/g 块四格都作为 BFS 入队起点，都记为"机器位置"
            for (int dy = 0; dy < 2; ++dy) for (int dx = 0; dx < 2; ++dx) {
                int x = gx + dx, y = gy + dy;
                if (x >= 0 && x < W && y >= 0 && y < H) {
                    if (visited.insert({ x,y }).second) queue.push_back({ x,y });
                }
            }
            // 机器主坐标计入 poweredMachines（后续 powerDraw 查询用）
            poweredMachines.insert(kv.first);
        }

        // 2) 标准 BFS，沿 '+' 电线 4 邻扩展；机器格（F/f/L/l/G/g）也是导体，会被
        //    自然发现并继续扩展（电可以穿过机器传导）。
        //    注意：不能像旧版那样把所有机器格无条件塞进 visited——那会让所有机器
        //    永远"已访问"，step 4 永远判定通电，电线形同虚设。
        auto isConductive = [&](int x, int y) -> bool {
            if (x < 0 || x >= W || y < 0 || y >= H) return false;
            char d = gameMap.getTileDisplay(x, y);
            if (d == '+') return true;
            if (d == 'F' || d == 'f' || d == 'L' || d == 'l' || d == 'G' || d == 'g') return true;
            if (d == 'X' || d == 'x' || d == 'W' || d == 'w' || d == 'R' || d == 'r') return true;
            return false;
        };

        static const int DX[4] = { 1,-1,0,0 };
        static const int DY[4] = { 0,0,1,-1 };
        size_t head = 0;
        while (head < queue.size()) {
            auto [cx, cy] = queue[head++];
            for (int d = 0; d < 4; ++d) {
                int nx = cx + DX[d], ny = cy + DY[d];
                if (!isConductive(nx, ny)) continue;
                if (!visited.insert({ nx,ny }).second) continue;
                queue.push_back({ nx,ny });
            }
        }

        // 4) 对于每个 F/L 机器，如果它的 4 格中有任何 1 格被 BFS visit 过，则认为通电
        for (auto& m : machineMeta) {
            if (m.type == 'F' || m.type == 'L' || m.type == 'G' ||
                m.type == 'X' || m.type == 'W' || m.type == 'R') {
                for (int dy = 0; dy < 2; ++dy) for (int dx = 0; dx < 2; ++dx) {
                    if (visited.count({ m.x + dx, m.y + dy })) {
                        poweredMachines.insert({ m.x, m.y });
                        goto next_machine;
                    }
                }
            }
        next_machine:;
        }

        // 5) 驱动所有发电机燃烧 + 注入 EU
        for (auto& kv : generators) {
            // 只有在 poweredMachines 里的发电机才真正燃烧（否则白烧煤——GTNH 里是无论连不连都烧，但这里简化为：连通才燃烧）
            bool connected = poweredMachines.count(kv.first);
            auto [inject, burning] = kv.second.updateTick();
            if (connected && burning) {
                globalEU = min(EU_MAX, globalEU + inject);
            }
        }
    }

    // 放置发电机（2×2）；返回 true 成功
    bool placeGenerator(int x, int y) {
        if (currentArea != Area::Home) return false;
        if (!gen_blueprint_unlocked) return false;
        if (player.coins < 100) return false;
        const int W = gameMap.getWidth(), H = gameMap.getHeight();
        if (x < 0 || x + 1 >= W || y < 0 || y + 1 >= H) return false;
        for (int dy = 0; dy < 2; ++dy) for (int dx = 0; dx < 2; ++dx) {
            if (gameMap.getTile(x + dx, y + dy).display != '.') return false;
        }
        player.coins -= 100;
        gameMap.getTile(x, y) = { 'G', "Power Generator", "Burns coal to make power", false, COLOR_RED };
        gameMap.getTile(x, y + 1) = { 'g', "Power Generator", "", false, COLOR_DARK_RED };
        gameMap.getTile(x + 1, y) = { 'g', "Power Generator", "", false, COLOR_DARK_RED };
        gameMap.getTile(x + 1, y + 1) = { 'g', "Power Generator", "", false, COLOR_DARK_RED };
        generators[{x, y}] = PowerGenerator{};
        machineMeta.push_back({ x, y, 'G', 0, 0, false });
        return true;
    }

    bool placeWire(int x, int y) {
        if (currentArea != Area::Home) return false;
        if (!wire_blueprint_unlocked) return false;
        if (player.coins < 5) return false;
        const int W = gameMap.getWidth(), H = gameMap.getHeight();
        if (x < 0 || x >= W || y < 0 || y >= H) return false;
        if (gameMap.getTile(x, y).display != '.') return false;
        player.coins -= 5;
        gameMap.getTile(x, y) = { '+', "Wire", "Conducts power", false, COLOR_BLUE };
        return true;
    }

    bool placeDecor(int x, int y, char display) {
        if (currentArea != Area::Home) return false;
        if (player.coins < 1) return false;
        const int W = gameMap.getWidth(), H = gameMap.getHeight();
        if (x < 0 || x >= W || y < 0 || y >= H) return false;
        if (gameMap.getTile(x, y).display != '.') return false;
        player.coins -= 1;
        int c = (display == '*') ? (rand() % 3 == 0 ? COLOR_YELLOW : (rand() % 2 ? COLOR_PURPLE : COLOR_GREEN))
                                : COLOR_DARK_GREEN;
        gameMap.getTile(x, y) = { display, display == '*' ? "Flower" : "Grass", "Decoration", true, c };
        return true;
    }

    // 放置 Crusher 破碎机（2×2，'X'/'x'，150c）
    bool placeCrusher(int x, int y) {
        if (currentArea != Area::Home) return false;
        if (!crusher_blueprint_unlocked) return false;
        if (player.coins < 150) return false;
        const int W = gameMap.getWidth(), H = gameMap.getHeight();
        if (x < 0 || x + 1 >= W || y < 0 || y + 1 >= H) return false;
        for (int dy = 0; dy < 2; ++dy) for (int dx = 0; dx < 2; ++dx) {
            if (gameMap.getTile(x + dx, y + dy).display != '.') return false;
        }
        player.coins -= 150;
        gameMap.getTile(x, y) = { 'X', "Crusher", "Ore crusher (4 EU/slot)", false, COLOR_YELLOW };
        gameMap.getTile(x, y + 1) = { 'x', "Crusher", "", false, COLOR_DARK_YELLOW };
        gameMap.getTile(x + 1, y) = { 'x', "Crusher", "", false, COLOR_DARK_YELLOW };
        gameMap.getTile(x + 1, y + 1) = { 'x', "Crusher", "", false, COLOR_DARK_YELLOW };
        crushers[{x, y}] = Crusher{};
        machineMeta.push_back({ x, y, 'X', 0, 0, false });
        return true;
    }

    // 放置 OreWasher 洗矿槽（2×2，'W'/'w'，120c）
    bool placeWasher(int x, int y) {
        if (currentArea != Area::Home) return false;
        if (!washer_blueprint_unlocked) return false;
        if (player.coins < 120) return false;
        const int W = gameMap.getWidth(), H = gameMap.getHeight();
        if (x < 0 || x + 1 >= W || y < 0 || y + 1 >= H) return false;
        for (int dy = 0; dy < 2; ++dy) for (int dx = 0; dx < 2; ++dx) {
            if (gameMap.getTile(x + dx, y + dy).display != '.') return false;
        }
        player.coins -= 120;
        gameMap.getTile(x, y) = { 'W', "Ore Washer", "Washes crushed ore (2 EU)", false, COLOR_CYAN };
        gameMap.getTile(x, y + 1) = { 'w', "Ore Washer", "", false, COLOR_DARK_CYAN };
        gameMap.getTile(x + 1, y) = { 'w', "Ore Washer", "", false, COLOR_DARK_CYAN };
        gameMap.getTile(x + 1, y + 1) = { 'w', "Ore Washer", "", false, COLOR_DARK_CYAN };
        washers[{x, y}] = OreWasher{};
        machineMeta.push_back({ x, y, 'W', 0, 0, false });
        return true;
    }

    // 放置 Centrifuge 离心机（2×2，'R'/'r'，200c）
    bool placeCentrifuge(int x, int y) {
        if (currentArea != Area::Home) return false;
        if (!centrifuge_blueprint_unlocked) return false;
        if (player.coins < 200) return false;
        const int W = gameMap.getWidth(), H = gameMap.getHeight();
        if (x < 0 || x + 1 >= W || y < 0 || y + 1 >= H) return false;
        for (int dy = 0; dy < 2; ++dy) for (int dx = 0; dx < 2; ++dx) {
            if (gameMap.getTile(x + dx, y + dy).display != '.') return false;
        }
        player.coins -= 200;
        gameMap.getTile(x, y) = { 'R', "Centrifuge", "Spins purified ore to dust (8 EU)", false, COLOR_PURPLE };
        gameMap.getTile(x, y + 1) = { 'r', "Centrifuge", "", false, COLOR_DARK_PURPLE };
        gameMap.getTile(x + 1, y) = { 'r', "Centrifuge", "", false, COLOR_DARK_PURPLE };
        gameMap.getTile(x + 1, y + 1) = { 'r', "Centrifuge", "", false, COLOR_DARK_PURPLE };
        centrifuges[{x, y}] = Centrifuge{};
        machineMeta.push_back({ x, y, 'R', 0, 0, false });
        return true;
    }

    // 通用机器查找（2×2-aware，4 格遍历，修复旧 findNearbyGenerator 盲区）
    bool findNearbyMachine(char type, int px, int py, pair<int, int>& outPos) const {
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

    // 找玩家邻近的发电机（用通用 findNearbyMachine 修复 2×2 盲区）
    bool findNearbyGenerator(int px, int py, pair<int, int>& outPos) const {
        return findNearbyMachine('G', px, py, outPos);
    }

    // 由全屏 UI 的 100ms ticker 统一调用：推进电网+高炉+车床+处理链机器
    void globalTick100ms() {
        tickPowerGrid();
        furnace.update(player, 100);
        // Lathe：先扣 2 EU 并把结果写进 hasPowerThisTick 字段，
        // 内部 update() 会根据它推进进度或显示 paused。
        // 车床坐标从 machineMeta 动态查找（不硬编码 {8,5}，否则旧存档 /
        // 重新生成的车床位置会查不到，powerDraw 永远返回 false）。
        pair<int, int> lathePos = { -1, -1 };
        for (auto& m : machineMeta) {
            if (m.type == 'L') { lathePos = { m.x, m.y }; break; }
        }
        bool machining = (lathe.getAnimState() == Lathe::Machining ||
                          lathe.getAnimState() == Lathe::Inserting);
        if (machining && lathePos.first >= 0) {
            lathe.hasPowerThisTick = powerDraw(2, lathePos);
        } else {
            lathe.hasPowerThisTick = true;
        }
        lathe.update(player, 100);

        // Crushers：每台破碎机每活跃槽单独 powerDraw(4 EU)，先到先得语义
        for (auto& kv : crushers) {
            auto& pos = kv.first;
            auto& cr = kv.second;
            for (int i = 0; i < cr.getSlotCount(); ++i) {
                auto& s = cr.getSlot(i);
                bool active = (s.phase != Crusher::IDLE && s.phase != Crusher::DONE_POUR);
                cr.setSlotPower(i, active ? powerDraw(4, pos) : true);
            }
            cr.update(player, 100);
        }
        // Washers：单槽，加工中扣 2 EU
        for (auto& kv : washers) {
            auto& ws = kv.second;
            bool processing = (ws.getAnimState() != OreWasher::Idle && ws.getAnimState() != OreWasher::Done);
            ws.setHasPowerThisTick(processing ? powerDraw(2, kv.first) : true);
            ws.update(player, 100);
        }
        // Centrifuges：单槽，加工中扣 8 EU
        for (auto& kv : centrifuges) {
            auto& cf = kv.second;
            bool processing = (cf.getAnimState() != Centrifuge::Idle && cf.getAnimState() != Centrifuge::Done);
            cf.setHasPowerThisTick(processing ? powerDraw(8, kv.first) : true);
            cf.update(player, 100);
        }
    }

public:
    // ===== public getters for UI =====
    int getGlobalEU() const { return globalEU; }
    int countBurningGenerators() const {
        int cnt = 0;
        for (auto& kv : generators) if (kv.second.isActive()) cnt++;
        return cnt;
    }
    int getTotalGenerators() const { return (int)generators.size(); }

    // ===== BuildUI：B 键打开，7×7 zoom grid + Radiobox 放置列表 =====
    void openBuildUI() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();
        string statusMsg = "Select a placeable on the left, then press PLACE to install it at your feet (center of grid).";
        Component placeMenu;
        Component buttonPlace;
        Component buttonClose;
        Component mainContainer;
        int placeSelection = 0;
        vector<string> placeEntries;

        auto rebuildPlaceEntries = [&]() {
            placeEntries.clear();
            placeEntries.push_back(string("  Generator (2x2, 100c)")
                + (gen_blueprint_unlocked ? "" : "  [LOCKED: buy BP first]"));
            placeEntries.push_back(string("  Wire (1x1, 5c)")
                + (wire_blueprint_unlocked ? "" : "  [LOCKED: buy BP first]"));
            placeEntries.push_back(string("  Crusher (2x2, 150c)")
                + (crusher_blueprint_unlocked ? "" : "  [LOCKED: buy BP first]"));
            placeEntries.push_back(string("  Washer (2x2, 120c)")
                + (washer_blueprint_unlocked ? "" : "  [LOCKED: buy BP first]"));
            placeEntries.push_back(string("  Centrifuge (2x2, 200c)")
                + (centrifuge_blueprint_unlocked ? "" : "  [LOCKED: buy BP first]"));
            placeEntries.push_back("  Flower (1c)");
            placeEntries.push_back("  Grass tuft (1c)");
            if (placeSelection >= (int)placeEntries.size()) placeSelection = (int)placeEntries.size() - 1;
            if (placeSelection < 0) placeSelection = 0;
        };
        rebuildPlaceEntries();
        placeMenu = Menu(&placeEntries, &placeSelection);

        auto tryPlace = [&]() {
            int px = player.x;
            int py = player.y;
            switch (placeSelection) {
            case 0:
                if (!gen_blueprint_unlocked) { statusMsg = "Generator blueprint required. Buy it from Trade > BUY."; return; }
                if (player.coins < 100) { statusMsg = "Need 100 coins."; return; }
                if (placeGenerator(px, py)) {
                    statusMsg = "Generator installed at (" + to_string(px) + "," + to_string(py) + ").";
                    rebuildPlaceEntries();
                } else { statusMsg = "Space is already occupied or impassable."; }
                break;
            case 1:
                if (!wire_blueprint_unlocked) { statusMsg = "Wire blueprint required. Buy it from Trade > BUY."; return; }
                if (player.coins < 5) { statusMsg = "Need 5 coins."; return; }
                if (placeWire(px, py)) {
                    statusMsg = "Wire laid at (" + to_string(px) + "," + to_string(py) + ").";
                    rebuildPlaceEntries();
                } else { statusMsg = "Cannot lay wire here."; }
                break;
            case 2:
                if (!crusher_blueprint_unlocked) { statusMsg = "Crusher blueprint required. Buy it from Trade > BUY."; return; }
                if (player.coins < 150) { statusMsg = "Need 150 coins."; return; }
                if (placeCrusher(px, py)) {
                    statusMsg = "Crusher installed at (" + to_string(px) + "," + to_string(py) + ").";
                    rebuildPlaceEntries();
                } else { statusMsg = "Space is already occupied or impassable."; }
                break;
            case 3:
                if (!washer_blueprint_unlocked) { statusMsg = "Washer blueprint required. Buy it from Trade > BUY."; return; }
                if (player.coins < 120) { statusMsg = "Need 120 coins."; return; }
                if (placeWasher(px, py)) {
                    statusMsg = "Washer installed at (" + to_string(px) + "," + to_string(py) + ").";
                    rebuildPlaceEntries();
                } else { statusMsg = "Space is already occupied or impassable."; }
                break;
            case 4:
                if (!centrifuge_blueprint_unlocked) { statusMsg = "Centrifuge blueprint required. Buy it from Trade > BUY."; return; }
                if (player.coins < 200) { statusMsg = "Need 200 coins."; return; }
                if (placeCentrifuge(px, py)) {
                    statusMsg = "Centrifuge installed at (" + to_string(px) + "," + to_string(py) + ").";
                    rebuildPlaceEntries();
                } else { statusMsg = "Space is already occupied or impassable."; }
                break;
            case 5:
                if (player.coins < 1) { statusMsg = "Need 1 coin."; return; }
                if (placeDecor(px, py, '*')) {
                    statusMsg = "Planted a flower at (" + to_string(px) + "," + to_string(py) + ").";
                } else { statusMsg = "Blocked."; }
                break;
            case 6:
                if (player.coins < 1) { statusMsg = "Need 1 coin."; return; }
                if (placeDecor(px, py, 'v')) {
                    statusMsg = "Planted grass tuft at (" + to_string(px) + "," + to_string(py) + ").";
                } else { statusMsg = "Blocked."; }
                break;
            }
        };

        buttonPlace = Button("PLACE", [&] { tryPlace(); });
        buttonClose = Button("CLOSE", [&] { screen.ExitLoopClosure()(); });

        auto gridRenderer = Renderer([&] {
            int cx = player.x, cy = player.y;
            Elements rows;
            for (int dy = -3; dy <= 3; ++dy) {
                Elements rowCells;
                for (int dx = -3; dx <= 3; ++dx) {
                    int x = cx + dx;
                    int y = cy + dy;
                    char d = gameMap.getTileDisplay(x, y);
                    Color col = Color::GrayDark;
                    bool isCenter = (dx == 0 && dy == 0);
                    if (d == '.' || d == '#') col = Color::GreenLight;
                    else if (d == '~') col = Color::BlueLight;
                    else if (d == '*') col = Color::MagentaLight;
                    else if (d == 'v') col = Color::Green;
                    else if (d == 't') col = Color::Green;
                    else if (d == '+' || d == 'G' || d == 'g') col = Color::YellowLight;
                    else if (d == 'F' || d == 'f') col = Color::RedLight;
                    else if (d == 'L' || d == 'l') col = Color::Magenta;
                    else if (d == 'X' || d == 'x') col = Color::Yellow;
                    else if (d == 'W' || d == 'w') col = Color::Cyan;
                    else if (d == 'R' || d == 'r') col = Color::MagentaLight;
                    else if (d == 'C') col = Color::Yellow;
                    else col = Color::White;
                    auto cell = text(string(1, d == '#' ? ' ' : d)) | color(col) | center;
                    cell = cell | size(WIDTH, EQUAL, 3) | size(HEIGHT, EQUAL, 1);
                    cell = cell | borderStyled(isCenter ? BorderStyle::DOUBLE : BorderStyle::LIGHT);
                    rowCells.push_back(cell);
                }
                rows.push_back(hbox(std::move(rowCells)));
            }
            auto title = text("  BUILD VIEW  7x7  (center = player feet)") | bold | color(Color::Cyan);
            auto gridBox = vbox(std::move(rows)) | borderDouble | center;
            return vbox({ separator(), title, separator(), gridBox }) | center;
        });

        auto infoRenderer = Renderer([&] {
            return vbox({
                hbox({ text(" Coins: ") | bold,
                        text(to_string(player.coins) + "c") | color(Color::Yellow) }),
                hbox({ text(" EU Pool: ") | bold,
                        text(to_string(globalEU) + " / 10000") | color(Color::Cyan) }),
                separator(),
                text(" Blueprints:") | bold,
                text("   Generator BP:  " + string(gen_blueprint_unlocked ? "OWNED" : "LOCKED [150c]"))
                    | color(gen_blueprint_unlocked ? Color::Green : Color::Red),
                text("   Wire BP:        " + string(wire_blueprint_unlocked ? "OWNED" : "LOCKED [50c]"))
                    | color(wire_blueprint_unlocked ? Color::Green : Color::Red),
                text("   Crusher BP:      " + string(crusher_blueprint_unlocked ? "OWNED" : "LOCKED [200c]"))
                    | color(crusher_blueprint_unlocked ? Color::Green : Color::Red),
                text("   Washer BP:      " + string(washer_blueprint_unlocked ? "OWNED" : "LOCKED [150c]"))
                    | color(washer_blueprint_unlocked ? Color::Green : Color::Red),
                text("   Centrifuge BP:  " + string(centrifuge_blueprint_unlocked ? "OWNED" : "LOCKED [300c]"))
                    | color(centrifuge_blueprint_unlocked ? Color::Green : Color::Red),
            }) | border;
        });

        auto buttonsBox = Container::Vertical({ buttonPlace, buttonClose });
        // 注意：buttonPlace/buttonClose 只能属于一个 Container，否则 FTXUI 焦点
        // 状态冲突会导致 Render() 返回空元素（按钮视觉消失）。这里 buttonsBox 是
        // 唯一的焦点容器；layout 只放 placeMenu + buttonsBox 一个整体。
        auto layout = Container::Vertical({ placeMenu, buttonsBox });
        mainContainer = Renderer(layout, [&, gridRenderer, infoRenderer, buttonsBox] {
            return vbox({
                text("            # BUILD MODE [B] #") | bold | color(Color::Yellow),
                separator(),
                hbox({
                    vbox({
                        text(" PLACEABLES") | bold | color(Color::Cyan),
                        separator(),
                        placeMenu->Render() | flex,
                    }) | size(WIDTH, GREATER_THAN, 42) | flex,
                    vbox({
                        infoRenderer->Render(),
                    }) | size(WIDTH, EQUAL, 34),
                }),
                separator(),
                gridRenderer->Render() | center,
                separator(),
                hbox({
                    filler(),
                    buttonsBox->Render() | size(WIDTH, EQUAL, 18),
                    filler(),
                }),
                separator(),
                text(statusMsg) | color(Color::Green),
                text("Tip: LEFT item pick type; PLACE puts it at the center (player). Decorations 1 coin each.")
                    | color(Color::GrayDark),
            }) | border | size(WIDTH, GREATER_THAN, 88);
        });

        // 加入 100ms ticker 以刷新 EU / 全局状态；ESC 直接退出 BuildUI
        atomic<bool> uiRunning{ true };
        mainContainer = mainContainer | CatchEvent([&](Event e) {
            if (e == Event::Custom) { globalTick100ms(); return true; }
            if (e == Event::Escape) {
                uiRunning.store(false);
                screen.ExitLoopClosure()();
                return true;
            }
            return false;
        });
        thread ticker([&screen, &uiRunning]() {
            while (uiRunning.load()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                if (!uiRunning.load()) break;
                try { screen.PostEvent(Event::Custom); } catch (...) { break; }
            }
        });

        screen.Loop(mainContainer);
        uiRunning.store(false);
        if (ticker.joinable()) ticker.join();

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        message = "& Build mode closed.";
        cls();
    }

    // ===== GeneratorPanel：E beside Generator 打开面板，投煤 / 看剩余 =====
    void openGeneratorPanel(int gx, int gy) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();
        string statusMsg = "Power Generator panel. Feed coal to keep the EU grid alive.";

        // 构造局部引用
        auto it = generators.find({ gx, gy });
        // 如果没找到，先创建一个空 PowerGenerator（避免 crash）
        if (it == generators.end()) {
            generators.insert({ {gx, gy}, PowerGenerator() });
            it = generators.find({ gx, gy });
        }
        PowerGenerator& genRef = it->second;
        Component buttonFeed = Button("ADD 1 COAL", [&] {
            if (genRef.feedCoal(player)) statusMsg = "+1 coal loaded into generator.";
            else statusMsg = "No coal in your backpack.";
        });
        Component buttonClose = Button("CLOSE", [&] { screen.ExitLoopClosure()(); });
        // 之前 Container::Vertical({...}) 是悬空表达式没赋值，按钮没进焦点链 →
        // Renderer 也只接收无子组件版本，导致按 Enter/Tab 完全不触发按钮。
        // 修复：把 Container 赋给变量并作为 Renderer 的子组件传入。
        auto buttons = Container::Vertical({ buttonFeed, buttonClose });

        Component view;
        view = Renderer(buttons, [&] {
            int remaining = genRef.burnEU;
            int loaded = genRef.loadedCoal;
            float burnPct = remaining >= 6400 ? 1.0f : (float)remaining / 6400.0f;
            if (burnPct < 0) burnPct = 0;
            const int barW = 40;
            int fill = (int)(burnPct * barW);
            string burnFill(fill, '='), burnSpace(barW - fill, ' ');
            string burning = genRef.isActive() ? "BURNING 8 EU/tick" : "IDLE";
            Color burnCol = genRef.isActive() ? Color::Green : Color::Red;

            int coalCount = player.getItemCount("coal");
            return vbox({
                text("     THERMAL GENERATOR @ (" + to_string(gx) + "," + to_string(gy) + ")")
                    | bold | color(Color::Yellow),
                separator(),
                hbox({ text("  Global EU Pool:  ") | bold,
                        text(to_string(globalEU) + " / 10000 EU") | color(Color::Cyan) }),
                separator(),
                text("  Fuel status") | bold | color(Color::Yellow),
                hbox({ text("  [" + burnFill + burnSpace + "] "),
                        text(to_string(remaining) + " EU / 6400 (1 coal)") }),
                hbox({ text("  Status: ") | bold,
                        text(burning) | color(burnCol) }),
                hbox({ text("  Coal loaded:  ") | bold, text("x" + to_string(loaded)) }),
                separator(),
                text("  Player inventory:") | bold,
                hbox({ text("  Coal on hand: ") | bold, text("x" + to_string(coalCount)) | color(Color::Cyan) }),
                separator(),
                vbox({
                    buttonFeed->Render() | flex,
                    buttonClose->Render() | flex,
                }) | center,
                separator(),
                text(statusMsg) | color(Color::Green),
                text("Tip: Each coal = 6400 EU (8 EU/tick x 800 ticks ~= 80s). Generator automatically connects to nearby wires.")
                    | color(Color::GrayDark),
            }) | border | size(WIDTH, GREATER_THAN, 72) | center;
        });
        atomic<bool> uiRunning{ true };
        view = view | CatchEvent([&](Event e) {
            if (e == Event::Custom) { globalTick100ms(); return true; }
            return false;
        });
        thread ticker([&screen, &uiRunning]() {
            while (uiRunning.load()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                if (!uiRunning.load()) break;
                try { screen.PostEvent(Event::Custom); } catch (...) { break; }
            }
        });

        screen.Loop(view);
        uiRunning.store(false);
        if (ticker.joinable()) ticker.join();

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        message = "& Generator panel closed.";
        cls();
    }

    // ===== CrusherUI：4 槽并行破碎机面板（镜像 FurnaceUI 简化版） =====
    void openCrusherUI(int cx, int cy) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();

        // 查找或创建 Crusher 实例
        auto it = crushers.find({ cx, cy });
        if (it == crushers.end()) {
            crushers.insert({ {cx, cy}, Crusher() });
            it = crushers.find({ cx, cy });
        }
        Crusher& crRef = it->second;

        string statusMsg = "Crusher at (" + to_string(cx) + "," + to_string(cy) + "). Load ore to crush.";
        int selRec = crRef.getSelectedRecipe();
        int selSlot = crRef.getFocusedSlot();
        vector<string> recipeLabels = crRef.getRecipeList();
        vector<string> slotLabels = { "Slot 0", "Slot 1", "Slot 2", "Slot 3" };

        Component slotTabs = Radiobox(&slotLabels, &selSlot);
        Component recipeList = Menu(&recipeLabels, &selRec);
        Component buttonLoad = Button("LOAD SLOT", [&] {
            if (crRef.loadSlot(player, selSlot)) {
                statusMsg = "Slot " + to_string(selSlot) + " loaded.";
                crRef.setFocusedSlot(selSlot);
            } else statusMsg = "Cannot load slot (busy or no ore).";
        });
        Component buttonCancel = Button("CANCEL SLOT", [&] {
            if (crRef.cancelSlot(selSlot)) statusMsg = "Slot " + to_string(selSlot) + " cancelled.";
            else statusMsg = "Cannot cancel (idle/done).";
        });
        Component buttonCollect = Button("COLLECT SLOT", [&] {
            if (crRef.collectSlot(player, selSlot)) statusMsg = "Collected from slot " + to_string(selSlot) + ".";
            else statusMsg = "Nothing to collect.";
        });
        Component buttonClose = Button("CLOSE", [&] { screen.ExitLoopClosure()(); });

        auto buttons = Container::Vertical({ buttonLoad, buttonCancel, buttonCollect, buttonClose });
        auto layout = Container::Vertical({ slotTabs, recipeList, buttons });

        Component view = Renderer(layout, [&] {
            // 同步选择状态到 crusher 实例
            crRef.setSelectedRecipe(selRec);
            crRef.setFocusedSlot(selSlot);
            // 4 槽进度条
            Elements bars;
            for (int i = 0; i < 4; ++i) {
                int pct = crRef.getProgressPct(i);
                const int barW = 30;
                int fill = (pct * barW) / 100;
                string barFill(fill, '#'), barSpace(barW - fill, '-');
                string phaseStr = "IDLE";
                auto& s = crRef.getSlot(i);
                if (s.phase == Crusher::STONING) phaseStr = "STONING";
                else if (s.phase == Crusher::MILLING) phaseStr = "MILLING";
                else if (s.phase == Crusher::DONE_POUR) phaseStr = "DONE";
                bool powered = crRef.getSlotPower(i);
                Color barCol = (s.phase == Crusher::DONE_POUR) ? Color::Green :
                               (s.phase == Crusher::IDLE) ? Color::GrayDark :
                               (powered ? Color::Yellow : Color::Red);
                bars.push_back(hbox({
                    text("S" + to_string(i) + " [" + phaseStr + "] "),
                    text("[" + barFill + barSpace + "] ") | color(barCol),
                    text(" " + to_string(pct) + "%"),
                    text(powered ? "" : "  [NO POWER]") | color(Color::Red),
                }));
            }
            // ASCII 动画（12 帧简版）
            int curFrame = 0;
            for (int i = 0; i < 4; ++i) {
                if (crRef.getSlot(i).phase != Crusher::IDLE) {
                    curFrame = crRef.getFrameIndex(i);
                    break;
                }
            }
            vector<string> art = {
                "   .---.      .---.      ",
                "  | STN |    | MILL |    ",
                "   '---'      '---'      ",
                "   /|\\        /|\\        ",
                "  / | \\      / | \\      ",
                "  \\\\|//      \\\\|//      ",
                "   \\|/        \\|/        ",
                "  [====]    [====]      ",
                "   \\\\\\\\      ////      ",
                "    \\\\\\\\    ////        ",
                "     ''''''''            ",
                "       CRUSHER            "
            };
            int af = curFrame % (int)art.size();
            Elements artElems;
            for (int i = 0; i < 6; ++i) {
                int idx = (af + i) % (int)art.size();
                artElems.push_back(text(art[idx]) | color(Color::Yellow));
            }

            return vbox({
                text("       # CRUSHER [X] #  @(" + to_string(cx) + "," + to_string(cy) + ")") | bold | color(Color::Yellow),
                separator(),
                hbox({
                    vbox({
                        text(" SLOTS") | bold | color(Color::Cyan),
                        separator(),
                        slotTabs->Render(),
                        separator(),
                        text(" RECIPES") | bold | color(Color::Cyan),
                        separator(),
                        recipeList->Render() | flex,
                    }) | size(WIDTH, GREATER_THAN, 42) | flex,
                    separator(),
                    vbox({
                        text(" PROGRESS") | bold | color(Color::Cyan),
                        separator(),
                        vbox(bars),
                        separator(),
                        text(" EU Pool: " + to_string(globalEU) + " / 10000") | color(Color::Cyan),
                        text(" Power: 4 EU/slot active") | color(Color::GrayDark),
                        separator(),
                        text(" ANIMATION") | bold | color(Color::Cyan),
                        vbox(artElems) | border,
                    }) | size(WIDTH, EQUAL, 40),
                }),
                separator(),
                buttons->Render() | center,
                separator(),
                text(statusMsg) | color(Color::Green),
            }) | border | size(WIDTH, GREATER_THAN, 90);
        });

        atomic<bool> uiRunning{ true };
        view = view | CatchEvent([&](Event e) {
            if (e == Event::Escape) { uiRunning.store(false); screen.ExitLoopClosure()(); return true; }
            if (e == Event::Custom) { return true; }
            return false;
        });

        thread ticker([&]() {
            while (uiRunning.load()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                if (!uiRunning.load()) break;
                globalTick100ms();
                try { screen.PostEvent(Event::Custom); } catch (...) { break; }
            }
        });

        screen.Loop(view);
        uiRunning.store(false);
        if (ticker.joinable()) ticker.join();

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        message = "& Crusher panel closed.";
        cls();
    }

    // ===== openWasherUI：单槽洗矿槽面板 =====
    void openWasherUI(int wx, int wy) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();
        auto it = washers.find({ wx, wy });
        if (it == washers.end()) {
            washers.insert({ {wx, wy}, OreWasher() });
            it = washers.find({ wx, wy });
        }
        OreWasher& wsRef = it->second;

        string statusMsg = "Washer at (" + to_string(wx) + "," + to_string(wy) + ").";
        int selRec = wsRef.getSelectedRecipe();
        vector<string> recipeLabels = wsRef.getRecipeList();

        Component recipeList = Radiobox(&recipeLabels, &selRec);
        Component buttonLoad = Button("LOAD CRUSHED", [&] {
            wsRef.setSelectedRecipe(selRec);
            if (wsRef.loadMaterials(player)) statusMsg = "Loaded. Washing...";
            else statusMsg = "Cannot load (busy or no crushed ore).";
        });
        Component buttonCollect = Button("COLLECT", [&] {
            if (wsRef.collect(player)) {
                statusMsg = "Collected purified ore.";
                if (!wsRef.getLastRareByproduct().empty()) {
                    statusMsg += " Rare: " + wsRef.getLastRareByproduct() + " x" + to_string(wsRef.getLastRareAmount());
                }
            } else statusMsg = "Nothing to collect.";
        });
        Component buttonClose = Button("CLOSE", [&] { screen.ExitLoopClosure()(); });

        auto buttons = Container::Vertical({ buttonLoad, buttonCollect, buttonClose });
        auto layout = Container::Vertical({ recipeList, buttons });

        Component view = Renderer(layout, [&] {
            wsRef.setSelectedRecipe(selRec);
            int pct = wsRef.getProgress();
            const int barW = 30;
            int fill = (pct * barW) / 100;
            string barFill(fill, '#'), barSpace(barW - fill, '-');
            string stateStr = "IDLE";
            Color barCol = Color::GrayDark;
            if (wsRef.getAnimState() == OreWasher::Washing) {
                stateStr = wsRef.getHasPowerThisTick() ? "WASHING" : "PAUSED (no power)";
                barCol = wsRef.getHasPowerThisTick() ? Color::Cyan : Color::Red;
            } else if (wsRef.getAnimState() == OreWasher::Done) {
                stateStr = "DONE";
                barCol = Color::Green;
            }
            // 简单 ASCII 动画（8 帧水流）
            int f = wsRef.getFrameIndex() % 8;
            vector<string> waterArt = {
                "  ~~~~~~~~~~~~~~~~~  ",
                "  ~  ~~~~~~~~~~  ~  ",
                "  ~    ~~~~~~    ~  ",
                "  ~      ~~      ~  ",
                "  ~    ~~~~~~    ~  ",
                "  ~  ~~~~~~~~~~  ~  ",
                "  ~~~~~~~~~~~~~~~~~  ",
                "    ORE  WASHER      "
            };
            Elements artElems;
            for (int i = 0; i < 4; ++i) {
                int idx = (f + i) % (int)waterArt.size();
                artElems.push_back(text(waterArt[idx]) | color(Color::Cyan));
            }

            return vbox({
                text("       # ORE WASHER [W] #  @(" + to_string(wx) + "," + to_string(wy) + ")") | bold | color(Color::Cyan),
                separator(),
                hbox({
                    vbox({
                        text(" RECIPES") | bold | color(Color::Cyan),
                        separator(),
                        recipeList->Render() | flex,
                    }) | size(WIDTH, GREATER_THAN, 42) | flex,
                    separator(),
                    vbox({
                        text(" STATUS") | bold | color(Color::Cyan),
                        separator(),
                        text("  State: " + stateStr) | color(barCol),
                        text("  [" + barFill + barSpace + "] ") | color(barCol),
                        text("  " + to_string(pct) + "%"),
                        separator(),
                        text("  EU Pool: " + to_string(globalEU) + " / 10000") | color(Color::Cyan),
                        text("  Power: 2 EU/tick when washing") | color(Color::GrayDark),
                        separator(),
                        text(" ANIMATION") | bold | color(Color::Cyan),
                        vbox(artElems) | border,
                    }) | size(WIDTH, EQUAL, 36),
                }),
                separator(),
                buttons->Render() | center,
                separator(),
                text(statusMsg) | color(Color::Green),
            }) | border | size(WIDTH, GREATER_THAN, 86);
        });

        atomic<bool> uiRunning{ true };
        view = view | CatchEvent([&](Event e) {
            if (e == Event::Escape) { uiRunning.store(false); screen.ExitLoopClosure()(); return true; }
            if (e == Event::Custom) { return true; }
            return false;
        });

        thread ticker([&]() {
            while (uiRunning.load()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                if (!uiRunning.load()) break;
                globalTick100ms();
                try { screen.PostEvent(Event::Custom); } catch (...) { break; }
            }
        });

        screen.Loop(view);
        uiRunning.store(false);
        if (ticker.joinable()) ticker.join();

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        message = "& Washer panel closed.";
        cls();
    }

    // ===== openCentrifugeUI：单槽离心机面板 =====
    void openCentrifugeUI(int rx, int ry) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();
        auto it = centrifuges.find({ rx, ry });
        if (it == centrifuges.end()) {
            centrifuges.insert({ {rx, ry}, Centrifuge() });
            it = centrifuges.find({ rx, ry });
        }
        Centrifuge& cfRef = it->second;

        string statusMsg = "Centrifuge at (" + to_string(rx) + "," + to_string(ry) + ").";
        int selRec = cfRef.getSelectedRecipe();
        vector<string> recipeLabels = cfRef.getRecipeList();

        Component recipeList = Radiobox(&recipeLabels, &selRec);
        Component buttonLoad = Button("LOAD PURIFIED", [&] {
            cfRef.setSelectedRecipe(selRec);
            if (cfRef.loadMaterials(player)) statusMsg = "Loaded. Spinning...";
            else statusMsg = "Cannot load (busy or no purified ore).";
        });
        Component buttonCollect = Button("COLLECT", [&] {
            if (cfRef.collect(player)) {
                statusMsg = "Collected dust.";
                if (!cfRef.getLastRareByproduct().empty()) {
                    statusMsg += " Rare: " + cfRef.getLastRareByproduct() + " x" + to_string(cfRef.getLastRareAmount());
                }
            } else statusMsg = "Nothing to collect.";
        });
        Component buttonClose = Button("CLOSE", [&] { screen.ExitLoopClosure()(); });

        auto buttons = Container::Vertical({ buttonLoad, buttonCollect, buttonClose });
        auto layout = Container::Vertical({ recipeList, buttons });

        Component view = Renderer(layout, [&] {
            cfRef.setSelectedRecipe(selRec);
            int pct = cfRef.getProgress();
            const int barW = 30;
            int fill = (pct * barW) / 100;
            string barFill(fill, '#'), barSpace(barW - fill, '-');
            string stateStr = "IDLE";
            Color barCol = Color::GrayDark;
            if (cfRef.getAnimState() == Centrifuge::Spinning) {
                stateStr = cfRef.getHasPowerThisTick() ? "SPINNING" : "PAUSED (no power)";
                barCol = cfRef.getHasPowerThisTick() ? Color::Magenta : Color::Red;
            } else if (cfRef.getAnimState() == Centrifuge::Done) {
                stateStr = "DONE";
                barCol = Color::Green;
            }
            // 旋转动画（10 帧）
            int f = cfRef.getFrameIndex() % 10;
            vector<string> spinArt = {
                "        .---.        ",
                "       |  |  |       ",
                "      |   |   |      ",
                "     |    |    |     ",
                "    |     |     |    ",
                "     |    |    |     ",
                "      |   |   |      ",
                "       |  |  |       ",
                "        '---'        ",
                "     CENTRIFUGE      "
            };
            Elements artElems;
            for (int i = 0; i < 5; ++i) {
                int idx = (f + i) % (int)spinArt.size();
                artElems.push_back(text(spinArt[idx]) | color(Color::MagentaLight));
            }

            return vbox({
                text("       # CENTRIFUGE [R] #  @(" + to_string(rx) + "," + to_string(ry) + ")") | bold | color(Color::MagentaLight),
                separator(),
                hbox({
                    vbox({
                        text(" RECIPES") | bold | color(Color::Cyan),
                        separator(),
                        recipeList->Render() | flex,
                    }) | size(WIDTH, GREATER_THAN, 42) | flex,
                    separator(),
                    vbox({
                        text(" STATUS") | bold | color(Color::Cyan),
                        separator(),
                        text("  State: " + stateStr) | color(barCol),
                        text("  [" + barFill + barSpace + "] ") | color(barCol),
                        text("  " + to_string(pct) + "%"),
                        separator(),
                        text("  EU Pool: " + to_string(globalEU) + " / 10000") | color(Color::Cyan),
                        text("  Power: 8 EU/tick when spinning") | color(Color::GrayDark),
                        separator(),
                        text(" ANIMATION") | bold | color(Color::Cyan),
                        vbox(artElems) | border,
                    }) | size(WIDTH, EQUAL, 36),
                }),
                separator(),
                buttons->Render() | center,
                separator(),
                text(statusMsg) | color(Color::Green),
            }) | border | size(WIDTH, GREATER_THAN, 86);
        });

        atomic<bool> uiRunning{ true };
        view = view | CatchEvent([&](Event e) {
            if (e == Event::Escape) { uiRunning.store(false); screen.ExitLoopClosure()(); return true; }
            if (e == Event::Custom) { return true; }
            return false;
        });

        thread ticker([&]() {
            while (uiRunning.load()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                if (!uiRunning.load()) break;
                globalTick100ms();
                try { screen.PostEvent(Event::Custom); } catch (...) { break; }
            }
        });

        screen.Loop(view);
        uiRunning.store(false);
        if (ticker.joinable()) ticker.join();

        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        message = "& Centrifuge panel closed.";
        cls();
    }

    void run() {
        introAnimation();
        int selectedSlot = selectSaveSlot();
        if (selectedSlot == -1) return;
        if (selectedSlot >= 0) {
            activeSaveSlot = selectedSlot == 0 ? 1 : selectedSlot;
            if (!loadGame(selectedSlot)) newGame();
        }
        else {
            newGame();
        }

        while (running) {
            cls();

            renderMap();
            showStatus();
            showInventory();
            showTutorial();
            showMessage();

            setcolor(COLOR_GREY);
            cout << "\n  [WASD Move] [E Interact] [C Backpack] [T Trade] [B Build] [F Hint] [H Help] [P Save] [L Load] [Q Quit]\n";
            setcolor(COLOR_RESET);

            int key = _getch();
            if (key == 224) {
                key = _getch();
                switch (key) {
                case 72: processKey('w'); break;
                case 80: processKey('s'); break;
                case 75: processKey('a'); break;
                case 77: processKey('d'); break;
                }
            }
            else {
                processKey(key);
            }
        }

        running = false;
        saveGame(0);

        cls();
        setcolor(COLOR_YELLOW);
        cout << "\n^_^ Thanks for playing, goodbye!\n";
        cout << "You reached level " << player.level << " with " << player.coins << " coins\n";
        setcolor(COLOR_RESET);
        cout << "\nPress any key to exit...";
        _getch();
    }
};

// ======================== main ========================
int main() {

    srand((unsigned)time(nullptr));

    try {
        ChemicalWorldGame game;
        game.run();
    }
    catch (const exception& e) {
        setcolor(COLOR_RED);
        cerr << "\nX Error occurred: " << e.what() << endl;
        setcolor(COLOR_RESET);
        system("pause");
        return 1;
    }
    
    return 0;
}


// ============== THE END ===================
//                 鸣谢
//               ??????????