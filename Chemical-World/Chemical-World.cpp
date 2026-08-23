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

class GameMap {
private:
    static constexpr int MAP_WIDTH = 30;
    static constexpr int MAP_HEIGHT = 20;
    Tile tiles[MAP_HEIGHT][MAP_WIDTH];

public:
    GameMap() {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                tiles[y][x] = { '.', "Stone", "Undiscovered ground", true, COLOR_GREY };
            }
        }
        
        vector<pair<string, char>> mineralTypes = {
            {"hematite", 'H'}, {"magnetite", 'M'}, {"bauxite", 'B'},
            {"coal", 'C'}, {"cassiterite", 'T'}, {"malachite", 'P'},
            {"chalcopyrite", 'U'}, {"gold_ore", 'G'}, {"silver_ore", 'S'}
        };

        // 生成隐藏矿点。矿脉会向相邻方向延伸，表面仍然显示为灰色点。
        for (int vein = 0; vein < 18; ++vein) {
            int x = rand() % MAP_WIDTH;
            int y = rand() % MAP_HEIGHT;
            auto mineral = mineralTypes[rand() % mineralTypes.size()];
            int richness = rand() % 3 + 1;
            int length = rand() % 3 + 1;
            for (int i = 0; i < length; ++i) {
                if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT &&
                    tiles[y][x].mineral.empty()) {
                    tiles[y][x].mineral = mineral.first;
                    tiles[y][x].display = '.';
                    tiles[y][x].name = "Hidden Deposit";
                    tiles[y][x].richness = richness;
                }
                if (rand() % 2 == 0) x += rand() % 3 - 1;
                else y += rand() % 3 - 1;
            }
        }

        tiles[5][5] = { '@', "Spawn Point", "Your starting point", true, COLOR_YELLOW };
    }

    Tile& getTile(int x, int y) {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
            static Tile wall = { '#', "Boundary", "World boundary", false, COLOR_RED };
            return wall;
        }
        return tiles[y][x];
    }

    bool isPassable(int x, int y) {
        return getTile(x, y).passable;
    }

    bool mineAt(PlayerData& player, int x, int y) {
        Tile& tile = getTile(x, y);
        // No mineral here
        if (tile.mineral.empty()) return false;

        // If hidden (default display '.'), first strike exposes the deposit
        if (tile.display == '.') {
            // Map mineral name to its intended display symbol (matching generation map)
            static std::map<std::string, char> symbols = {
                {"hematite", 'H'}, {"magnetite", 'M'}, {"bauxite", 'B'},
                {"coal", 'C'}, {"cassiterite", 'T'}, {"malachite", 'P'},
                {"chalcopyrite", 'U'}, {"gold_ore", 'G'}, {"silver_ore", 'S'}
            };
            char hint = '.';
            auto it = symbols.find(tile.mineral);
            if (it != symbols.end()) hint = it->second;
            else if (!tile.mineral.empty()) hint = toupper(tile.mineral[0]);

            tile.display = hint;
            tile.hits = 1;
            tile.name = tile.mineral; // expose proper name
            cout << " You exposed a deposit: " << tile.mineral << " (" << tile.hits << "/3)." << endl;
            return true;
        }

        // Subsequent strikes accumulate hits; third strike yields ore based on richness
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

    int getWidth() const { return MAP_WIDTH; }
    int getHeight() const { return MAP_HEIGHT; }

    void interact(PlayerData& player, int x, int y) {
        Tile& tile = getTile(x, y);
        cout << (tile.mineral.empty() ? " Nothing to interact with." :
            " Use WASD to strike this deposit three times.") << endl;
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
        auto layout = Container::Vertical({
            Container::Horizontal({
                recipeList,
                Container::Vertical({
                    Container::Horizontal({ buttonOreMinus, buttonOrePlus }),
                    Container::Horizontal({ buttonFuelMinus, buttonFuelPlus }),
                    buttonLoad,
                    buttonBlower,
                    buttonCancel,
                    buttonClose,
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

// ======================== 游戏主程序 ========================
class ChemicalWorldGame {
private:
    PlayerData player;
    GameMap gameMap;
    BlastFurnace furnace;
    bool running = true;
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

    // 交易系统
    void trade() {
        vector<string> items;
        vector<int> prices;
        vector<int> quantities;
        int selected = 0;
        bool trading = true;

        for (auto& item : player.inventory) {
            if (item.quantity > 0) {
                int price = getItemPrice(item.name);
                if (price > 0) {
                    items.push_back(item.name);
                    prices.push_back(price);
                    quantities.push_back(item.quantity);
                }
            }
        }
        items.push_back("Back");

        while (trading) {
            cls();
            setcolor(COLOR_YELLOW);
            cout << "========================================\n";
            cout << "|          $ Trading Market          |\n";
            cout << "========================================\n\n";
            setcolor(COLOR_RESET);

            cout << "Coins: " << player.coins << "\n\n";
            cout << "Sellable Items:\n";

            for (size_t i = 0; i < items.size(); i++) {
                if (i == (size_t)selected) {
                    setcolor(COLOR_YELLOW);
                    cout << " >> " << items[i];
                    if (i < items.size() - 1) {
                        cout << " x" << quantities[i] << " (Price: " << prices[i] << ")";
                    }
                    cout << " <<\n";
                    setcolor(COLOR_RESET);
                }
                else {
                    cout << "    " << items[i];
                    if (i < items.size() - 1) {
                        cout << " x" << quantities[i] << " (Price: " << prices[i] << ")";
                    }
                    cout << "\n";
                }
            }

            cout << "\n[UP][DOWN] Select, [ENTER] Sell (all), [ESC] Back\n";

            int key = _getch();
            if (key == 224) {
                key = _getch();
                if (key == 72) selected = (selected - 1 + items.size()) % items.size();
                else if (key == 80) selected = (selected + 1) % items.size();
            }
            else if (key == 27) {
                trading = false;
            }
            else if (key == 13 && selected < (int)(items.size() - 1)) {
                int qty = quantities[selected];
                int price = prices[selected];
                if (qty > 0 && player.removeItem(items[selected], qty)) {
                    int total = price * qty;
                    player.coins += total;
                    player.exp += qty * 5;
                    message = "! Sold " + items[selected] + " x" + to_string(qty) + ", earned " + to_string(total) + " coins!";
                    checkTutorialProgress("trade");
                    checkLevelUp();
                    trading = false;
                }
                else {
                    message = "X Cannot sell this item";
                }
            }
        }
    }

    int getItemPrice(const string& item) {
        static map<string, int> prices = {
            {"water", 3}, {"hematite", 5}, {"magnetite", 5}, {"bauxite", 4},
            {"cassiterite", 5}, {"malachite", 6}, {"chalcopyrite", 6},
            {"gold_ore", 20}, {"silver_ore", 12}, {"coal", 8},
            {"steel", 30}, {"glass", 15}, {"sand", 2},
            {"iron_ingot", 12}, {"alloy", 45}
        };
        auto it = prices.find(item);
        return it != prices.end() ? it->second : 0;
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

    void renderMap() {
        setcolor(COLOR_CYAN);
        cout << "==================================================\n";
        cout << "|                   World Map                    |\n";
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
                    cout << tile.display;
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
        cout << "~=Water ";
        setcolor(COLOR_DARK_RED);
        cout << "H=Hematite M=Magnetite B=Bauxite ";
        cout << "T=Cassiterite P=Malachite U=Chalcopyrite ";
        cout << "G=Gold A=Silver ";
        setcolor(COLOR_DARK_YELLOW);
        cout << "C=Coal\n";
        setcolor(COLOR_RESET);
    }

    void showStatus() {
        setcolor(COLOR_YELLOW);
        cout << "\n==================================================\n";
        cout << "|  " << player.name << " | Lv." << player.level;
        cout << " | Coins: " << player.coins;
        cout << " | Exp: " << player.exp;
        cout << string(30 - (player.name.length() + to_string(player.level).length() +
            to_string(player.coins).length() + to_string(player.exp).length()), ' ');
        cout << "|\n";
        cout << "|  Position: (" << player.x << ", " << player.y << ")";
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
        cout << "  E     - Interact (collect/use)\n";
        cout << "  C     - Crafting menu\n";
        cout << "  T     - Trading market\n";
        cout << "  F     - Blast furnace\n";
        cout << "  H     - Show help\n";
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
            gameMap.interact(player, player.x, player.y);
            checkTutorialProgress("collect");
            return;
        case 'c': case 'C':
            craft();
            return;
        case 't': case 'T':
            trade();
            return;
        case 'f': case 'F':
            openFurnaceUI();
            return;
        case 'h': case 'H':
            cls();
            showHelp();
            cout << "\nPress any key to continue...";
            _getch();
            return;
        case 'q': case 'Q':
            running = false;
            return;
        default:
            return;
        }

        if (gameMap.mineAt(player, newX, newY)) {
            message = "Mining deposit: strike " + to_string(gameMap.getTile(newX, newY).hits) + "/3";
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
        player.name = "Chemist";
        player.addItem("water", 5, "basic", 3);
        player.addItem("coal", 3, "fuel", 8);
        player.addItem("sand", 3, "material", 2);

        while (running) {
            cls();

            renderMap();
            showStatus();
            showInventory();
            showTutorial();
            showMessage();

            setcolor(COLOR_GREY);
            cout << "\n  [WASD Move] [E Interact] [C Craft] [T Trade] [F Furnace] [H Help] [Q Quit]\n";
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