// Chemical-world.cpp
// 使用 FTXUI 6.1.9 实现图形界面
// 特别警告：目前 !!!游戏内容!!! 请使用全英文+基础ascii以避免乱码，注释可以使用中文

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <stdio.h>
#include <map>
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
        {"steel_spring", 30}, {"steel_bolt", 12}, {"steel_wire", 35}
    };
    auto it = prices.find(name);
    return it != prices.end() ? it->second : 0;
}

// ======================== 高炉系统 ========================
class BlastFurnace {
private:
    bool isRunning = false;
    int progress = 0;
    int maxProgress = 100;
    int temperature = 20;
    int targetTemperature = 1400;
    int temperatureTolerance = 75;
    int heatTime = 0;
    int requiredHeatTime = 5000;
    bool loaded = false;
    string currentRecipe = "";
    int fuelCount = 0;
    int oreCount = 0;
    int resultCount = 0;
    vector<string> logs;

    struct Recipe {
        string name;
        string oreName;
        string result;
        int oreRequired;
        int fuelRequired;
        int resultAmount;
        int duration; // 秒
    };

    vector<Recipe> recipes = {
        {"Steel Making", "hematite", "steel", 2, 1, 1, 5},
        {"Steel Making", "magnetite", "steel", 2, 1, 1, 5},
        {"Aluminum Smelting", "bauxite", "aluminum", 2, 1, 1, 5},
        {"Tin Smelting", "cassiterite", "tin", 2, 1, 1, 5},
        {"Copper Smelting", "malachite", "copper", 2, 1, 1, 5},
        {"Copper Smelting", "chalcopyrite", "copper", 2, 1, 1, 5},
        {"Gold Smelting", "gold_ore", "gold_ingot", 2, 1, 1, 5},
        {"Silver Smelting", "silver_ore", "silver_ingot", 2, 1, 1, 5}
    };

    int selectedRecipe = 0;
    int loadedRecipe = 0;

public:
    BlastFurnace() {}

    void addLog(const string& msg) {
        logs.push_back(msg);
        if (logs.size() > 20) logs.erase(logs.begin());
    }

    string getStatus() {
        if (isRunning) {
            if (!loaded) return "ready to load";
            if (temperature > targetTemperature + temperatureTolerance) return "OVERHEATED";
            if (temperature >= targetTemperature - temperatureTolerance) return "holding temperature";
            return "heating - use blower";
        }
        return "free";
    }

    int getProgress() { return progress; }
    int getMaxProgress() { return maxProgress; }
    bool isActive() { return isRunning; }
    int getTemperature() { return temperature; }
    int getTargetTemperature() { return targetTemperature; }
    int getTemperatureTolerance() { return temperatureTolerance; }
    int getHeatTime() { return heatTime; }
    int getRequiredHeatTime() { return requiredHeatTime; }
    bool isLoaded() { return loaded; }

    void selectRecipe(int index) {
        if (!isRunning && index >= 0 && index < (int)recipes.size()) {
            selectedRecipe = index;
        }
    }

    string getRecipeName() {
        return recipes[selectedRecipe].name;
    }

    string getRecipeInfo() {
        Recipe& r = recipes[selectedRecipe];
        return r.name + " (" + r.oreName + "): " + to_string(r.oreRequired) + " ore + " +
            to_string(r.fuelRequired) + " fuel -> " + r.result + " x" + to_string(r.resultAmount);
    }

    bool canLoad(PlayerData& player) {
        Recipe& r = recipes[selectedRecipe];
        return !isRunning &&
            player.hasItem(r.oreName, r.oreRequired) &&
            player.hasItem("coal", r.fuelRequired);
    }

    int findRecipe(const string& oreName, int oreAmount, int fuelAmount) {
        for (int i = 0; i < (int)recipes.size(); ++i) {
            Recipe& r = recipes[i];
            if (r.oreName == oreName && r.oreRequired == oreAmount &&
                r.fuelRequired == fuelAmount) return i;
        }
        return -1;
    }

    bool canLoad(PlayerData& player, const string& oreName, int oreAmount, int fuelAmount) {
        return !isRunning && findRecipe(oreName, oreAmount, fuelAmount) >= 0 &&
            player.hasItem(oreName, oreAmount) && player.hasItem("coal", fuelAmount);
    }

    void loadMaterials(PlayerData& player) {
        if (isRunning) return;
        Recipe& r = recipes[selectedRecipe];
        if (!canLoad(player)) {
            addLog("X NOT ENOUGH MATERIALS");
            return;
        }

        player.removeItem(r.oreName, r.oreRequired);
        player.removeItem("coal", r.fuelRequired);

        isRunning = true;
        loaded = true;
        progress = 0;
        temperature = 200;
        heatTime = 0;
        currentRecipe = r.name;
        loadedRecipe = selectedRecipe;
        oreCount = r.oreRequired;
        fuelCount = r.fuelRequired;
        resultCount = r.resultAmount;

        addLog("Loaded " + to_string(r.oreRequired) + " ore and " +
            to_string(r.fuelRequired) + " coal. Start blowing air!");
    }

    bool loadMaterials(PlayerData& player, const string& oreName, int oreAmount, int fuelAmount) {
        int recipeIndex = findRecipe(oreName, oreAmount, fuelAmount);
        if (isRunning || recipeIndex < 0 || !player.hasItem(oreName, oreAmount) ||
            !player.hasItem("coal", fuelAmount)) {
            addLog("X Charge does not match any recipe or materials are missing.");
            return false;
        }
        selectedRecipe = recipeIndex;
        loadMaterials(player);
        return true;
    }

    bool canStart(PlayerData& player) { return canLoad(player); }
    void start(PlayerData& player) { loadMaterials(player); }

    bool blowAir() {
        if (!isRunning || !loaded) return false;
        temperature = min(1800, temperature + 90);
        return true;
    }

    bool update(PlayerData& player, int elapsedMs = 100) {
        if (!isRunning || !loaded) return false;

        temperature = max(20, temperature - 12);
        if (temperature > targetTemperature + temperatureTolerance) {
            isRunning = false;
            loaded = false;
            progress = 0;
            temperature = 20;
            addLog("X Furnace overheated! The batch was lost.");
            return false;
        }

        if (temperature >= targetTemperature - temperatureTolerance &&
            temperature <= targetTemperature + temperatureTolerance) {
            heatTime += elapsedMs;
            progress = min(maxProgress, heatTime * maxProgress / requiredHeatTime);
        }

        if (heatTime >= requiredHeatTime) {
            isRunning = false;
            loaded = false;
            progress = 100;
            temperature = 20;

            Recipe& r = recipes[loadedRecipe];
            player.addItem(r.result, resultCount, "product", 20);
            player.exp += 15;
            addLog("OK! " + r.name + " completed! You got " + r.result + " x" + to_string(resultCount));

            // 检查升级
            int needed = 100 + player.level * 20;
            while (player.exp >= needed) {
                player.exp -= needed;
                player.level++;
                addLog("Level Up! Current level: " + to_string(player.level));
                needed = 100 + player.level * 20;
            }

            return true;
        }
        return false;
    }

    void cancel() {
        if (isRunning) {
            isRunning = false;
            loaded = false;
            progress = 0;
            temperature = 20;
            addLog("Smelting cancelled");
        }
    }

    vector<string> getLogs() { return logs; }

    vector<string> getRecipeList() {
        vector<string> result;
        for (auto& r : recipes) {
            result.push_back(r.name + " [" + r.oreName + "] -> " + r.result + " x" + to_string(r.resultAmount));
        }
        return result;
    }

    int getSelectedRecipe() { return selectedRecipe; }
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
        if (animState == Machining) {
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
            tiles[5][5] = { 'F', "Blast Furnace", "Installed machine", false, COLOR_RED };
            tiles[5][12] = { 'C', "Car", "Travel to the wasteland", true, COLOR_YELLOW };
            tiles[5][6] = { '.', "Grass", "Home grass", true, COLOR_GREEN };
            // 车床放在 (x=5, y=8)，与高炉(5,5)/汽车(12,5) 无相邻格重叠，E 键无歧义
            tiles[8][5] = { 'L', "Lathe", "Machining facility", false, COLOR_PURPLE };
            tiles[6][5] = { '.', "Grass", "Home grass", true, COLOR_GREEN };
            tiles[7][5] = { '.', "Grass", "Home grass", true, COLOR_GREEN };
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
                    && tile.display != 'F' && tile.display != 'L';
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
    string statusMessage = "WELCOME TO THE FURNACE!";

    Component mainContainer;
    Component recipeList;
    Component buttonLoad;
    Component buttonPut;
    Component buttonPutDone;
    Component buttonBlower;
    Component buttonOreMinus;
    Component buttonOrePlus;
    Component buttonFuelMinus;
    Component buttonFuelPlus;
    Component buttonCancel;
    Component buttonClose;
    Component progressBar;
    Component logViewer;

    // 保持配方列表的生命周期，使 Radiobox 引用有效
    std::vector<std::string> recipeNames;
    std::vector<std::string> inputNames;

    int selectedInput = 0;
    int oreQuantity = 2;
    int fuelQuantity = 1;
    bool needRefresh = false;
    bool putting = false;

public:
    FurnaceUI(BlastFurnace& f, PlayerData& p, ScreenInteractive& s)
        : furnace(f), player(p), screen(s) {
        setupUI();
    }

    void setupUI() {
        // 创建配方列表 - 使用 Radiobox
        inputNames = { "hematite", "magnetite", "bauxite", "cassiterite",
            "malachite", "chalcopyrite", "gold_ore", "silver_ore" };
        recipeList = Radiobox(&inputNames, &selectedInput);

        // 绑定选择事件
        recipeList |= CatchEvent([&](Event event) {
            if (event == Event::ArrowUp || event == Event::ArrowDown) {
                needRefresh = true;
                return true;
            }
            return false;
            });

        buttonPut = Button("PUT", [&] {
            putting = true;
            needRefresh = true;
            // 进入投料视图后把焦点交给 "RETURN TO FURNACE"，
            // 这样直接按 Enter 即可返回主界面。
            buttonPutDone->TakeFocus();
            });
        buttonPutDone = Button("RETURN TO FURNACE", [&] {
            putting = false;
            needRefresh = true;
            // 返回主界面后把焦点交回 PUT，方便继续操作。
            buttonPut->TakeFocus();
            });

        buttonLoad = Button("LOAD MATERIALS", [&] {
            if (furnace.canLoad(player, inputNames[selectedInput], oreQuantity, fuelQuantity)) {
                furnace.loadMaterials(player, inputNames[selectedInput], oreQuantity, fuelQuantity);
                statusMessage = "Charge loaded. Keep the temperature in the green zone!";
                needRefresh = true;
            }
            else {
                statusMessage = "Need the selected recipe's ore and coal.";
                needRefresh = true;
            }
            });

        buttonOreMinus = Button("ORE -", [&] { oreQuantity = max(1, oreQuantity - 1); });
        buttonOrePlus = Button("ORE +", [&] { oreQuantity = min(8, oreQuantity + 1); });
        buttonFuelMinus = Button("COAL -", [&] { fuelQuantity = max(1, fuelQuantity - 1); });
        buttonFuelPlus = Button("COAL +", [&] { fuelQuantity = min(8, fuelQuantity + 1); });

        buttonBlower = Button("BLOW AIR", [&] {
            if (furnace.blowAir()) {
                statusMessage = "Whoosh! Temperature increased.";
            }
            else {
                statusMessage = "Load materials before using the blower.";
            }
            needRefresh = true;
            });

        // 取消按钮
        buttonCancel = Button("CANCEL", [&] {
            furnace.cancel();
            statusMessage = "Cancelled";
            needRefresh = true;
            });

        // 关闭按钮
        buttonClose = Button("CLOSE", [&] {
            running = false;
            screen.ExitLoopClosure()();
            });

        // 进度条组件
        progressBar = Renderer([&] {
            int prog = furnace.getProgress();
            int maxProg = furnace.getMaxProgress();
            int filled = (prog * 20) / maxProg;

            string bar = "[";
            for (int i = 0; i < 30; i++) {
                if (i < (prog * 30) / maxProg) bar += "#";
                else bar += ".";
            }
            bar += "]";

            string statusText = furnace.isActive() ? "..." : "|| free";
            string percent = to_string(prog) + "%";

            return hbox({ text("  "), text(bar) | color(Color::Green),
                text("  "), text(statusText) | bold, text("  "), text(percent) });
            });

        // 日志查看器
        logViewer = Renderer([&] {
            auto logs = furnace.getLogs();
            Elements elems;
            for (auto& log : logs) {
                elems.push_back(text(log));
            }
            if (elems.empty()) {
                elems.push_back(text("(no logs)"));
            }
            return vbox(elems) | border | size(HEIGHT, LESS_THAN, 8);
            });

        // 主布局
        // 说明：buttonPutDone 之前只被 mainContainer 的渲染函数通过 ->Render() 手动绘制，
        // 并未挂载到 layout 组件树中，因此它永远收不到键盘/鼠标事件 —— 表现为按下 PUT
        // 进入投料视图后，"RETURN TO FURNACE" 按钮点不动、无法返回主界面。
        // 修复：把 buttonPutDone 挂到组件树里，并用 Maybe 按视图切换按钮的可见/可聚焦状态，
        // 避免在某一视图里 Tab 到不可见按钮（例如投料时误触不可见的 CLOSE）。
        auto showMain = [this] { return !putting; };
        auto showPut = [this] { return  putting; };

        auto layout = Container::Vertical({
            Container::Horizontal({
                recipeList,
                Container::Vertical({
                    Container::Horizontal({ buttonOreMinus, buttonOrePlus }),
                    Container::Horizontal({ buttonFuelMinus, buttonFuelPlus }),
                    Maybe(buttonLoad,    showMain),
                    Maybe(buttonPut,     showMain),
                    Maybe(buttonBlower,  showMain),
                    Maybe(buttonCancel,  showMain),
                    Maybe(buttonClose,   showMain),
                    Maybe(buttonPutDone, showPut),
                }),
            }),
            progressBar,
            logViewer,
            });

        mainContainer = Renderer(layout, [&] {
            // 获取当前配方信息
            string recipeName = inputNames[selectedInput];
            string recipeInfo = "Charge: " + recipeName + " x" + to_string(oreQuantity) +
                " + coal x" + to_string(fuelQuantity);
            string status = furnace.getStatus();

            // 库存信息
            int selectedOre = player.getItemCount(recipeName);
            int coal = player.getItemCount("coal");
            int steel = player.getItemCount("steel");
            int temperature = furnace.getTemperature();
            int target = furnace.getTargetTemperature();
            int tolerance = furnace.getTemperatureTolerance();
            string temperatureText = to_string(temperature) + " C   target " +
                to_string(target - tolerance) + " - " + to_string(target + tolerance) + " C";
            Color temperatureColor = temperature > target + tolerance ? Color::Red :
                (temperature >= target - tolerance ? Color::Green : Color::Yellow);
            string heatText = to_string(furnace.getHeatTime() / 1000) + " / " +
                to_string(furnace.getRequiredHeatTime() / 1000) + " sec";

            if (putting) {
                return vbox({
                    text("              PUT MATERIALS") | bold | color(Color::Magenta),
                    separator(),
                    text("Select the ore and coal charge for the next batch."),
                    text("Ore:  " + recipeName + " x" + to_string(oreQuantity)),
                    hbox({ buttonOreMinus->Render(), buttonOrePlus->Render() }),
                    text("Coal: coal x" + to_string(fuelQuantity)),
                    hbox({ buttonFuelMinus->Render(), buttonFuelPlus->Render() }),
                    text("PUT only stages the charge. Return and press LOAD to validate it."),
                    buttonPutDone->Render(),
                    }) | border | size(WIDTH, GREATER_THAN, 60);
            }

            return vbox({
                text("        BLAST FURNACE  //  CONTROL DECK") | bold | color(Color::Yellow),
                separator(),

                // 状态和配方信息
                hbox({
                    text("Status: ") | bold,
                    text(status) | bold | color(furnace.isActive() ? Color::Cyan : Color::GrayDark),
                }),
                hbox({
                    text("Current Recipe: ") | bold,
                    text(recipeName) | color(Color::Cyan),
                }),
                hbox({
                    text("Recipe Info: ") | bold,
                    text(recipeInfo) | color(Color::Green),
                }),
                separator(),

                hbox({ text(" CHARGE  ") | bold | color(Color::Magenta),
                    text(recipeName + "  ") | color(Color::Red), text(to_string(selectedOre) + "   "),
                    text("Coal  ") | color(Color::Yellow), text(to_string(coal) + "   "),
                    text("Steel  ") | color(Color::Cyan), text(to_string(steel)) }),
                separator(),

                hbox({ text(" TEMPERATURE  ") | bold, text(temperatureText) | color(temperatureColor) }),
                gauge(static_cast<float>(min(temperature, 1800)) / 1800.0f) |
                    color(temperatureColor) | border,
                hbox({ text(" HOLD TIME     ") | bold, text(heatText),
                    text(furnace.isLoaded() ? "   Keep clicking BLOW AIR" : "   Load a batch to begin") |
                        color(Color::GrayDark) }),
                progressBar->Render() | border,

                hbox({
                    text(" INPUT: ") | bold,
                    text(recipeName + " x" + to_string(oreQuantity) + "  +  coal x" +
                        to_string(fuelQuantity)) | color(Color::Magenta),
                }),
                hbox({
                    buttonLoad->Render() | flex,
                    buttonPut->Render() | flex,
                    buttonBlower->Render() | flex,
                    buttonCancel->Render() | flex,
                    buttonClose->Render() | flex,
                }),

                separator(),

                // 日志
                text(" EVENT LOG") | bold | color(Color::Yellow),
                logViewer->Render() | flex,

                // 消息
                text(statusMessage) | color(Color::Yellow),
                text("Tip: temperature cools continuously. Green means the batch is being refined.") |
                    color(Color::GrayDark),
                }) | border | size(WIDTH, GREATER_THAN, 78);
            });

        mainContainer |= CatchEvent([&](Event event) {
            if (event == Event::Custom) {
                if (furnace.update(player)) {
                    statusMessage = "Steel is ready! Batch completed.";
                }
                needRefresh = true;
                return true;
            }
            return false;
            });
    }

    Component getComponent() {
        return mainContainer;
    }

    bool isRunning() { return running; }

    void refresh() {
        if (needRefresh) {
            needRefresh = false;
        }
    }
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
    string statusMessage = "WELCOME TO THE LATHE!";

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

        // 进度条
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
            return hbox({ text("  "), text(bar) | color(Color::Green),
                text("  "), text(percent) });
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
                return vbox({
                    text("        LATHE  //  MACHINING") | bold | color(Color::Red),
                    separator(),
                    artBox->Render(),
                    progressBar->Render() | border,
                    hbox({ buttonCancel->Render() | flex }),
                    text(statusMessage) | color(Color::Yellow),
                    }) | border | size(WIDTH, GREATER_THAN, 60);
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
                hbox({ text(" Status: ") | bold, text(lathe.getStatus()) | color(Color::Cyan) }),
                hbox({ text(" Mold:   ") | bold, text(lathe.getMoldInfo()) | color(Color::Green) }),
                separator(),
                hbox({ text(" STEEL  ") | bold | color(Color::Magenta),
                    text("steel  ") | color(Color::Cyan), text(to_string(steel)) }),
                separator(),
                artBox->Render(),
                separator(),
                hbox({ buttonLoad->Render() | flex, buttonSelect->Render() | flex, buttonClose->Render() | flex }),
                separator(),
                text(" EVENT LOG") | bold | color(Color::Yellow),
                logViewer->Render() | flex,
                text(statusMessage) | color(Color::Yellow),
                text("Tip: pick a mold, then LOAD. Animation plays inside this window.") | color(Color::GrayDark),
                }) | border | size(WIDTH, GREATER_THAN, 78);
            });

        // tick 钩子：100ms 一次，同时推进进度与动画帧
        mainContainer |= CatchEvent([&](Event event) {
            if (event == Event::Custom) {
                if (lathe.update(player)) {
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
// 图形化交易界面：左侧列表显示背包所有物品（含不可卖标注），
// 右侧详情 + SELL ONE / SELL ALL / SELL EVERYTHING / CLOSE。
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
    Component buttonClose;
    Component detailBox;

    int selected = 0;
    vector<string> entries;        // Menu 显示文本
    vector<string> names;          // 对应物品名
    vector<int>    quantities;     // 对应数量
    vector<int>    prices;         // 对应单价

    void rebuild() {
        entries.clear();
        names.clear();
        quantities.clear();
        prices.clear();
        for (auto& item : player.inventory) {
            if (item.quantity > 0) {
                int price = itemPrice(item.name);
                names.push_back(item.name);
                quantities.push_back(item.quantity);
                prices.push_back(price);
                string line = "  " + item.name + "  x" + to_string(item.quantity);
                line += price > 0 ? ("   [" + to_string(price) + "c]") : "   [not sellable]";
                entries.push_back(line);
            }
        }
        if (entries.empty()) {
            entries.push_back("  (backpack empty)");
            names.push_back("");
            quantities.push_back(0);
            prices.push_back(0);
        }
        if (selected >= (int)entries.size()) selected = (int)entries.size() - 1;
        if (selected < 0) selected = 0;
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

    bool validSelection() {
        if (selected < 0 || selected >= (int)names.size()) return false;
        if (names[selected].empty()) return false;
        return prices[selected] > 0 && quantities[selected] > 0;
    }

public:
    TradeUI(PlayerData& p, ScreenInteractive& s) : player(p), screen(s) {
        setupUI();
    }

    void setupUI() {
        rebuild();
        menu = Menu(&entries, &selected);

        buttonSellOne = Button("SELL ONE", [&] {
            if (!validSelection()) { statusMessage = "Cannot sell this."; return; }
            string name = names[selected];
            if (player.removeItem(name, 1)) {
                int total = prices[selected];
                player.coins += total;
                player.exp += 5;
                statusMessage = "Sold " + name + " x1, +" + to_string(total) + "c";
                rebuild();
                checkLevelUpInternal();
            }
            });

        buttonSellAll = Button("SELL ALL", [&] {
            if (!validSelection()) { statusMessage = "Cannot sell this."; return; }
            string name = names[selected];
            int qty = quantities[selected];
            if (player.removeItem(name, qty)) {
                int total = prices[selected] * qty;
                player.coins += total;
                player.exp += qty * 5;
                statusMessage = "Sold " + name + " x" + to_string(qty) + ", +" + to_string(total) + "c";
                rebuild();
                checkLevelUpInternal();
            }
            });

        buttonSellEverything = Button("SELL EVERYTHING", [&] {
            // 先收集要卖的，避免边遍历边改 inventory
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
            rebuild();
            checkLevelUpInternal();
            });

        buttonClose = Button("CLOSE", [&] {
            running = false;
            screen.ExitLoopClosure()();
            });

        detailBox = Renderer([&] {
            if (selected < 0 || selected >= (int)names.size() || names[selected].empty()) {
                return vbox({ text("  No item selected.") }) | border;
            }
            string name = names[selected];
            int qty = quantities[selected];
            int price = prices[selected];
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
            });

        auto layout = Container::Vertical({
            Container::Horizontal({
                menu | flex,
                Container::Vertical({
                    buttonSellOne,
                    buttonSellAll,
                    buttonSellEverything,
                    buttonClose,
                }),
            }),
        });

        mainContainer = Renderer(layout, [&] {
            return vbox({
                text("            $ TRADING MARKET $") | bold | color(Color::Yellow),
                separator(),
                hbox({ text(" Coins: ") | bold, text(to_string(player.coins)) | color(Color::Yellow) }),
                separator(),
                hbox({
                    vbox({
                        text(" YOUR BACKPACK") | bold | color(Color::Cyan),
                        separator(),
                        menu->Render() | flex,
                    }) | flex,
                    vbox({
                        detailBox->Render(),
                        separator(),
                        buttonSellOne->Render() | flex,
                        buttonSellAll->Render() | flex,
                        buttonSellEverything->Render() | flex,
                        separator(),
                        buttonClose->Render() | flex,
                    }),
                }) | flex,
                separator(),
                text(statusMessage) | color(Color::Green),
                text("Tip: UP/DOWN select, SELL ONE/ALL for one item, SELL EVERYTHING clears all sellables.") | color(Color::GrayDark),
            }) | border | size(WIDTH, GREATER_THAN, 78);
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

    struct TutorialStep {
        string title;
        string description;
        string action;
        bool completed = false;
    };


    vector<TutorialStep> tutorials = {
        {"Move & Explore", "Use WASD keys to move on the map\nTry moving to nearby water sources or mining spots", "move", false},
        {"Collect Resources", "Move into a hidden deposit and strike it 3 times with WASD\nFind hematite, magnetite, or coal first", "collect", false},
        {"Craft Items", "Press C to open the crafting menu\nTry crafting steel (Hematite x2 + Coal x1)", "craft", false},
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
        output << "CHEMICAL_WORLD_SAVE 2\n";
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
        return output.good();
    }

    bool loadGame(int slot) {
        ifstream input(savePath(slot));
        string header;
        int version = 0;
        if (!input || !(input >> header >> version) || header != "CHEMICAL_WORLD_SAVE" || version != 2) return false;
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
        lathe = Lathe{};  // 读档后清空车床运行态（运行态不持久化，与高炉一致）
        return gameMap.load(input);
    }

    void newGame() {
        player = PlayerData{};
        gameMap = GameMap{};
        furnace = BlastFurnace{};
        currentArea = Area::Home;
        gameMap.generate(currentArea);
        player.x = 8;
        player.y = 5;
        player.name = "Chemist";
        player.addItem("water", 5, "basic", 3);
        player.addItem("coal", 3, "fuel", 8);
        player.addItem("sand", 3, "material", 2);
        for (auto& tutorial : tutorials) tutorial.completed = false;
        tutorialActive = true;
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

    // 合成系统
    void craft() {
        vector<string> options = { "Steel (Hematite x2 + Coal x1)", "Glass (Sand x2 + Coal x1)", "Back" };
        int selected = 0;
        bool crafting = true;

        while (crafting) {
            cls();
            setcolor(COLOR_CYAN);
            cout << "========================================\n";
            cout << "|          % Crafting Menu           |\n";
            cout << "========================================\n\n";
            setcolor(COLOR_RESET);

            for (size_t i = 0; i < options.size(); i++) {
                if (i == (size_t)selected) {
                    setcolor(COLOR_YELLOW);
                    cout << " >> " << options[i] << " <<\n";
                    setcolor(COLOR_RESET);
                }
                else {
                    cout << "    " << options[i] << "\n";
                }
            }

            cout << "\nInventory:\n";
            cout << "  Hematite: " << player.getItemCount("hematite") << "\n";
            cout << "  Coal: " << player.getItemCount("coal") << "\n";
            cout << "  Sand: " << player.getItemCount("sand") << "\n";
            cout << "\n[UP][DOWN] Select, [ENTER] Confirm, [ESC] Back\n";

            int key = _getch();
            if (key == 224) {
                key = _getch();
                if (key == 72) selected = (selected - 1 + options.size()) % options.size();
                else if (key == 80) selected = (selected + 1) % options.size();
            }
            else if (key == 27) {
                crafting = false;
            }
            else if (key == 13) {
                if (selected == 0) {
                    if (player.hasItem("hematite", 2) && player.hasItem("coal", 1)) {
                        player.removeItem("hematite", 2);
                        player.removeItem("coal", 1);
                        player.addItem("steel", 1, "product", 30);
                        player.exp += 10;
                        message = "! Crafted steel!";
                        checkTutorialProgress("craft");
                        checkLevelUp();
                        crafting = false;
                    }
                    else {
                        message = "X Not enough materials! Need Hematite x2 + Coal x1";
                    }
                }
                else if (selected == 1) {
                    if (player.hasItem("sand", 2) && player.hasItem("coal", 1)) {
                        player.removeItem("sand", 2);
                        player.removeItem("coal", 1);
                        player.addItem("glass", 1, "product", 15);
                        player.exp += 8;
                        message = "! Crafted glass!";
                        checkLevelUp();
                        crafting = false;
                    }
                    else {
                        message = "X Not enough materials! Need Sand x2 + Coal x1";
                    }
                }
                else {
                    crafting = false;
                }
            }
        }
    }

    // 交易系统（FTXUI 图形化界面）
    void trade() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        auto screen = ScreenInteractive::Fullscreen();
        TradeUI tradeUI(player, screen);
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

        setcolor(COLOR_GREEN);
        cout << "\n  ";
        setcolor(COLOR_RESET);
        cout << "P=Player ";
        setcolor(COLOR_GREEN);
        cout << ".=Grass ";
        setcolor(COLOR_DARK_GREEN);
        cout << "T=Tree ";
        setcolor(COLOR_GREY);
        cout << "S=Stone ";
        setcolor(COLOR_BLUE);
        cout << "~=Salt ~  ";
        cout << "=Rock ";
        if (currentArea == Area::Home) cout << "F=Furnace L=Lathe C=Car a=Animal ";
        else cout << "O=Cave entrance ";
        setcolor(COLOR_DARK_RED);
        cout << "H=Hematite M=Magnetite B=Bauxite ";
        cout << "T=Cassiterite P=Malachite U=Chalcopyrite ";
        cout << "G=Gold A=Silver ";
        setcolor(COLOR_DARK_YELLOW);
        cout << "C=Coal\n";
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
        cout << "  E     - Interact nearby (mine/enter/use)\n";
        cout << "  C     - Crafting menu\n";
        cout << "  T     - Trading market\n";
        cout << "  F     - Hint only; use E beside furnace at home\n";
        cout << "  H     - Show help\n";
        cout << "  F5    - Save to active manual slot\n";
        cout << "  F9    - Load active manual slot\n";
        cout << "  Q     - Quit game\n";
        setcolor(COLOR_RESET);
    }

    void processKey(int key) {
        int newX = player.x, newY = player.y;

        switch (key) {
        case 'w': case 'W': newY--; break;
        case 's': case 'S': newY++; break;
        case 'a': case 'A': newX--; break;
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
            craft();
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
        case 1005:
            if (saveGame(activeSaveSlot)) message = "Game saved to Manual Slot " + to_string(activeSaveSlot) + ".";
            else message = "X Could not save the game.";
            return;
        case 1009:
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

public:
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
            cout << "\n  [WASD Move] [E Interact] [C Craft] [T Trade] [F Hint] [H Help] [F5 Save] [F9 Load] [Q Quit]\n";
            setcolor(COLOR_RESET);

            int key = _getch();
            if (key == 224) {
                key = _getch();
                switch (key) {
                case 72: processKey('w'); break;
                case 80: processKey('s'); break;
                case 75: processKey('a'); break;
                case 77: processKey('d'); break;
                case 63: processKey(1005); break;
                case 67: processKey(1009); break;
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

// ======================== 主函数 ========================
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