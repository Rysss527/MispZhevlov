#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <array>
#include <string_view>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <thread>
#include <chrono>
#include "Constants.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "GameContext.h"
#include "MainMenuScreen.h"
#include "TutorialScreen.h"
#include "RankingScreen.h"
#include "BattlefieldScreen.h"
#include "PauseScreen.h"
#include "ProgressScreen.h"
#include "GamePlayer.h"

Player player;
#include "GameOverScreen.h"
#include "WinScreen.h"
#include "LevelUpScreen.h"
#include "GameCompleteScreen.h"
#include "LoadoutScreenWrapper.h"
#include "CrystalManager.h"
#include "ScreenBase.h"
#include "ResourceManager.h"
#include "LoadingScreen.h"
#include <thread>
#include <atomic>
#include <chrono>

using namespace DefenseSynth;

enum class ScreenType : uint8_t {
    MainMenu,
    Tutorial,
    Rankings,
    Loadout,
    Battlefield,
    Pause,
    Progress,
    GameOver,
    Win,
    LevelUp,
    GameComplete,
    Invalid,
    Count
};

[[nodiscard]] inline ScreenType parseScreenType(const std::string_view name) noexcept {
    static const std::array<std::pair<std::string_view, ScreenType>, 11> mapping{{
        {"mainMenu", ScreenType::MainMenu},
        {"tutorial", ScreenType::Tutorial},
        {"rankings", ScreenType::Rankings},
        {"loadout", ScreenType::Loadout},
        {"battlefield", ScreenType::Battlefield},
        {"pause", ScreenType::Pause},
        {"progress", ScreenType::Progress},
        {"gameover", ScreenType::GameOver},
        {"win", ScreenType::Win},
        {"levelUp", ScreenType::LevelUp},
        {"gameComplete", ScreenType::GameComplete}
    }};
    
    const auto it = std::find_if(mapping.begin(), mapping.end(),
        [name](const auto& pair) { return pair.first == name; });
    return it != mapping.end() ? it->second : ScreenType::Invalid;
}

class GameApplication final {
private:
    sf::RenderWindow window;
    std::array<std::unique_ptr<DefenseSynth::Screen>, static_cast<size_t>(ScreenType::Count)> screens{};
    std::unique_ptr<BattlefieldScreen> battlefield;
    ScreenType currentScreen{ScreenType::MainMenu};
    ScreenType previousScreen{ScreenType::Invalid};
    std::string viewString;
    LoadoutScreenWrapper* loadoutCache{nullptr};
    sf::Clock deltaClock;
    std::atomic<bool> shouldTerminate{false};

public:
    GameApplication() 
        : window(sf::VideoMode(GameConstants::WINDOW_WIDTH, GameConstants::WINDOW_HEIGHT),
                "Plants vs Zombies Defense Synth", sf::Style::Close | sf::Style::Titlebar),
          viewString("mainMenu") {
        window.setFramerateLimit(GameConstants::FPS);
        window.setVerticalSyncEnabled(true);
        
        LoadingScreen loadingScreen(window);
        sf::Clock loadingClock;
        sf::Clock frameClock;
        
        std::atomic<float> loadProgress(0.0f);
        std::atomic<bool> loadingComplete(false);
        std::string currentStatus = "Initializing...";
        shouldTerminate = false;
        
        auto& rm = ResourceManager::getInstance();
        
        rm.loadTextureWithRetry("assets/images/22.png", "app_icon", 1);
        if (rm.isTextureLoaded("app_icon")) {
            const sf::Image icon = rm.getTexture("app_icon").copyToImage();
            window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
        }
        
        std::thread loadingThread([&]() {
            if (shouldTerminate) return;
            
            loadProgress = 0.05f;
            currentStatus = "Initializing resources...";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (shouldTerminate) return;
            
            loadProgress = 0.1f;
            currentStatus = "Loading textures...";
            auto start = std::chrono::high_resolution_clock::now();
            bool texturesLoaded = rm.preloadTextures();
            auto texDuration = std::chrono::high_resolution_clock::now() - start;
            if (shouldTerminate) return;
            
            loadProgress = 0.5f;
            currentStatus = "Loading audio...";
            start = std::chrono::high_resolution_clock::now();
            bool soundsLoaded = rm.preloadSounds();
            auto sndDuration = std::chrono::high_resolution_clock::now() - start;
            if (shouldTerminate) return;
            
            loadProgress = 0.85f;
            currentStatus = "Loading fonts...";
            bool fontsLoaded = rm.preloadFonts();
            if (shouldTerminate) return;
            
            loadProgress = 0.95f;
            currentStatus = "Finalizing...";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (shouldTerminate) return;
            
            loadProgress = 1.0f;
            currentStatus = "Complete!";
            loadingComplete = true;
        });
        
        while (!loadingComplete && window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    shouldTerminate = true;
                    loadingComplete = true;
                    window.close();
                    break;
                }
            }
            
            if (shouldTerminate) {
                break;
            }
            
            float deltaTime = frameClock.restart().asSeconds();
            loadingScreen.setProgress(loadProgress.load());
            loadingScreen.setStatus(currentStatus);
            loadingScreen.update(deltaTime);
            loadingScreen.draw();
            
            sf::sleep(sf::milliseconds(16));
        }
        
        loadingThread.join();
        
        if (!shouldTerminate && window.isOpen()) {
            sf::sleep(sf::milliseconds(200));
            viewString.reserve(16);
            initializeScreens();
        }
    }

    void initializeScreens() {
        try {
            screens[static_cast<size_t>(ScreenType::MainMenu)] = std::make_unique<MainMenuScreen>(window);
            screens[static_cast<size_t>(ScreenType::Tutorial)] = std::make_unique<TutorialScreen>(window);
            screens[static_cast<size_t>(ScreenType::Rankings)] = std::make_unique<RankingScreen>(window);
            screens[static_cast<size_t>(ScreenType::Pause)] = std::make_unique<PauseScreen>(window);
            screens[static_cast<size_t>(ScreenType::Progress)] = std::make_unique<ProgressScreen>(window);
            screens[static_cast<size_t>(ScreenType::GameOver)] = std::make_unique<GameOverScreen>(window);
            screens[static_cast<size_t>(ScreenType::Win)] = std::make_unique<WinScreen>(window);
            screens[static_cast<size_t>(ScreenType::LevelUp)] = std::make_unique<LevelUpScreen>(window);
            auto loadout = std::make_unique<LoadoutScreenWrapper>(window);
            loadoutCache = loadout.get();
            screens[static_cast<size_t>(ScreenType::Loadout)] = std::move(loadout);
            
        } catch (const std::exception& ex) {
            for (auto& screen : screens) {
                screen.reset();
            }
            loadoutCache = nullptr;
            throw std::runtime_error("Failed to initialize screens: " + std::string(ex.what()));
        }
    }

    void run() {
        if (shouldTerminate) {
            return;
        }
        
        SoundManager& soundManager = SoundManager::getInstance();
        
        while (window.isOpen() && !shouldTerminate) {
            sf::Time dt = deltaClock.restart();
            float deltaTime = dt.asSeconds();
            soundManager.updateMusic();
            render(deltaTime);
        }
        
        cleanup();
    }
    
    void cleanup() {
        SoundManager::getInstance().stopMusic();
        for (auto& screen : screens) {
            if (screen) {
                screen.reset();
            }
        }
        if (battlefield) {
            battlefield.reset();
        }
        loadoutCache = nullptr;
    }
    

    void render(float deltaTime) {
        handleScreenTransition();
        renderCurrentScreen(deltaTime);
    }
    
    void handleScreenTransition() noexcept {
        const auto newScreen = parseScreenType(viewString);
        if (newScreen == currentScreen) return;
        
        previousScreen = currentScreen;
        currentScreen = newScreen;
        if (currentScreen == ScreenType::Loadout && battlefield) {
            battlefield.reset();
        }
        if (currentScreen == ScreenType::MainMenu && previousScreen == ScreenType::Pause && battlefield) {
            battlefield.reset();
        }
    }
    
    void renderCurrentScreen(float deltaTime) {
        switch (currentScreen) {
            case ScreenType::Invalid:
                window.close();
                return;
            case ScreenType::Battlefield:
                renderBattlefield(deltaTime);
                return;
            case ScreenType::Pause:
                renderPauseScreen();
                return;
            case ScreenType::GameComplete:
                ensureGameCompleteScreen();
                [[fallthrough]];
            default:
                renderRegularScreen(deltaTime);
                if (viewString == "exit") {
                    window.close();
                    return;
                }
                return;
        }
    }

    void renderBattlefield(float deltaTime) {
        try {
            if (!battlefield) {
                createBattlefield();
            }
            battlefield->renderScreen(window, viewString, deltaTime);
        } catch (...) {
            fallbackToMainMenu();
        }
    }

    void renderRegularScreen(float deltaTime) {
        try {
            const auto idx = static_cast<size_t>(currentScreen);
            if (auto& screen = screens[idx]; screen) {
                screen->renderScreen(window, viewString);
            }
        } catch (...) {
            fallbackToMainMenu();
        }
    }

    void ensureGameCompleteScreen() {
        try {
            constexpr auto idx = static_cast<size_t>(ScreenType::GameComplete);
            if (!screens[idx]) {
                screens[idx] = std::make_unique<GameCompleteScreen>(window);
            }
        } catch (...) {
            fallbackToMainMenu();
        }
    }
    
    void renderPauseScreen() {
        try {
            const auto idx = static_cast<size_t>(ScreenType::Pause);
            if (auto& screen = screens[idx]; screen) {
                screen->renderScreen(window, viewString);
            }
        } catch (...) {
            fallbackToMainMenu();
        }
    }

    void fallbackToMainMenu() noexcept {
        viewString = "mainMenu";
        currentScreen = ScreenType::MainMenu;
    }

    void createBattlefield() {
        if (!loadoutCache) {
            throw std::runtime_error("Loadout cache not available for battlefield creation");
        }
        
        if (previousScreen == ScreenType::Pause && battlefield) {
            return;
        }
        
        try {
            battlefield = std::make_unique<BattlefieldScreen>(
                window,
                StateManager::getInstance()->getSelectedUnits(),
                loadoutCache->getUpgradeLevels(),
                loadoutCache->getDifficultyValue()
            );
            
            StateManager::getInstance()->startBattleSession();
        } catch (const std::exception& ex) {
            battlefield.reset();
            throw std::runtime_error("Failed to create battlefield: " + std::string(ex.what()));
        }
    }

    ~GameApplication() noexcept {
        shouldTerminate = true;
        if (window.isOpen()) {
            window.close();
        }
        try {
            cleanup();
        } catch (...) {
        }
        try {
            StateManager::getInstance()->save();
        } catch (...) {
        }
        try {
            GameContext::cleanup();
        } catch (...) {
        }
    }
};

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, 
                   [[maybe_unused]] HINSTANCE hPrevInstance, 
                   [[maybe_unused]] LPSTR lpCmdLine, 
                   [[maybe_unused]] int nCmdShow)
#else
int main()
#endif
{
    try {
        SoundManager::getInstance().initialize();
        StateManager::getInstance()->load();
        CrystalManager::getInstance().setCrystalCount(
            StateManager::getInstance()->getCrystals());
        GameContext::getInstance();
        
        GameApplication{}.run();
        
        DefenseSynth::ResourceManager::cleanup();
        StateManager::getInstance()->save();
        
        return 0;
        
    } catch (const std::exception& e) {
        DefenseSynth::ResourceManager::cleanup();
        #ifdef _WIN32
        MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
        #endif
        return -1;
    }
}
