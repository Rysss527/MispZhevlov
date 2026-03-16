#pragma once
#include <SFML/Window/Cursor.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "TowerFactory.h"
#include "EnemyFactory.h"
#include "Resource.h"
#include "GameCursor.h"
#include "Store.h"
#include "GameMonitor.h"
#include "DefenseUnitFactory.h"
#include "GamePlayer.h"
#include "CollisionManager.h"
#include "SoundManager.h"
#include "Constants.h"
#include "GameCursor.h"
#include "LoadoutScreen.h"
#include "CardCollection.h"
#include "CrystalManager.h"
#include "UITheme.h"
#include "StateManager.h"
#include "ResourceManager.h"

extern Player player;

class Screen
{
public:
    sf::Texture bgImg;
    sf::Sprite background;
    Screen(sf::RenderWindow &window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/background.png", "background", 1);
        background.setTexture(rm.getTexture("background"));
        background.setTextureRect(sf::IntRect(0, 0, GameConstants::BATTLEFIELD_WIDTH, GameConstants::BATTLEFIELD_HEIGHT));
        window.setVerticalSyncEnabled(true);
    }

    virtual void renderScreen(sf::RenderWindow &window, std::string &currentScreen) = 0;
    virtual ~Screen() {}
};

#include <vector>

class MainMenuScreen : public Screen
{
private:
    static inline std::vector<std::string> menuTracks;
    static inline int currentTrack = 0;
    sf::Font font;
    sf::Text playButtonText;
    sf::Text exitButtonText;

public:
    MainMenuScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/homescreen.png", "homescreen", 1);
        background.setTexture(rm.getTexture("homescreen"));

        if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
            rm.loadFont("assets/fonts/new.ttf", "arial");
        }
        
        playButtonText.setFont(rm.getFont("arial"));
        playButtonText.setCharacterSize(36);
        playButtonText.setFillColor(sf::Color::White);
        playButtonText.setStyle(sf::Text::Bold);
        
        exitButtonText.setFont(rm.getFont("arial"));
        exitButtonText.setString("Exit");
        exitButtonText.setCharacterSize(42);
        exitButtonText.setFillColor(sf::Color::Red);
        exitButtonText.setStyle(sf::Text::Bold);

        
        if (menuTracks.empty()) {
            menuTracks = UIConstants::Audio::MENU_TRACKS;
        }
        
        SoundManager::getInstance()->setPlaylist(menuTracks);
    }
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        SoundManager::getInstance()->updateMusic();
        
        
        StateManager* sm = StateManager::getInstance();
        
        
        sm->load();
        
        
        
        bool hasActiveSave = sm->isBattleSessionActive();
        
        
        playButtonText.setString(hasActiveSave ? "Resume" : "Play");
        
        
        sf::Vector2u windowSize = window.getSize();
        float scaleX = windowSize.x / 1300.0f;
        float scaleY = windowSize.y / 900.0f;
        float buttonCenterX = (66 * scaleX + 635 * scaleX) / 2.0f;
        float buttonCenterY = (356 * scaleY + 501 * scaleY) / 2.0f;
        
        sf::FloatRect textBounds = playButtonText.getLocalBounds();
        playButtonText.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        playButtonText.setPosition(buttonCenterX, buttonCenterY);
        
        sf::FloatRect exitBounds = exitButtonText.getLocalBounds();
        exitButtonText.setOrigin(exitBounds.width, exitBounds.height);
        exitButtonText.setPosition(windowSize.x - 20 * scaleX, windowSize.y - 20 * scaleY);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                float mouseX = static_cast<float>(event.mouseButton.x);
                float mouseY = static_cast<float>(event.mouseButton.y);
                
                
                if (mouseX >= 66 * scaleX && mouseX <= 635 * scaleX && mouseY >= 356 * scaleY && mouseY <= 501 * scaleY)
                {
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    
                    if (hasActiveSave) {
                        
                        StateManager::getInstance()->load();
                        currentScreen = "battlefield";
                    } else {
                        
                        StateManager* sm = StateManager::getInstance();
                        CrystalManager& cm = CrystalManager::getInstance();
                        
                        sm->reset();
                        cm.reset();
                        
                        sm->save();
                        currentScreen = "loadout";
                    }
                }
                
                else if (mouseX >= 66 * scaleX && mouseX <= 635 * scaleX && mouseY >= 526 * scaleY && mouseY <= 669 * scaleY)
                {
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    currentScreen = "tutorial";
                }
                
                else if (mouseX >= 66 * scaleX && mouseX <= 635 * scaleX && mouseY >= 695 * scaleY && mouseY <= 839 * scaleY)
                {
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    currentScreen = "rankings";
                }
                else if (mouseX >= windowSize.x - 100 * scaleX && mouseX <= windowSize.x - 10 * scaleX && 
                         mouseY >= windowSize.y - 60 * scaleY && mouseY <= windowSize.y - 10 * scaleY)
                {
                    window.close();
                }
            }
        }

        window.clear();
        window.draw(background);
        window.draw(exitButtonText);
        window.display();
    }
};

class TutorialScreen : public Screen
{
public:
    TutorialScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/instructions.png", "instructions", 1);
        background.setTexture(rm.getTexture("instructions"));
    }
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                float mouseX = static_cast<float>(event.mouseButton.x);
                float mouseY = static_cast<float>(event.mouseButton.y);
                sf::Vector2u windowSize = window.getSize();
                
                
                float scaleX = windowSize.x / 1300.0f;
                float scaleY = windowSize.y / 900.0f;

                
                if (mouseX >= 927 * scaleX && mouseX <= 1277 * scaleX && mouseY >= 810 * scaleY && mouseY <= 889 * scaleY)
                {
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    currentScreen = "mainMenu";
                }
            }
        }
        window.clear();
        window.draw(background);
        window.display();
    }
};

class RankingScreen : public Screen
{
private:
    sf::Font font;
    sf::Text highScoresText;

public:
    RankingScreen(sf::RenderWindow& window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/leaderboard.png", "leaderboard", 1);
        background.setTexture(rm.getTexture("leaderboard"));
        if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
            rm.loadFont("assets/fonts/new.ttf", "arial");
        }
        highScoresText.setFont(rm.getFont("arial"));
        highScoresText.setCharacterSize(32);
        highScoresText.setFillColor(sf::Color::White);
        highScoresText.setPosition(450, 300);
    }

    void renderScreen(sf::RenderWindow& window, std::string& currentScreen)
    {
        std::ifstream highScoresFile("highscores.txt");
        std::string highScoresStr;
        if (highScoresFile.is_open())
        {
            std::string line;
            int count = 0;
            while (std::getline(highScoresFile, line) && count < 10)
            {
                size_t spacePos = line.find_last_of(' ');
                if (spacePos != std::string::npos) {
                    std::string name = line.substr(0, spacePos);
                    std::string score = line.substr(spacePos + 1);
                    highScoresStr += name + " Score: " + score + "\n";
                } else {
                    highScoresStr += line + "\n";
                }
                count++;
            }
            highScoresFile.close();
        }
        else
        {
            highScoresStr = "No highscores yet!";
        }
        highScoresText.setString(highScoresStr);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                float mouseX = static_cast<float>(event.mouseButton.x);
                float mouseY = static_cast<float>(event.mouseButton.y);
                sf::Vector2u windowSize = window.getSize();
                
                
                float scaleX = windowSize.x / 1300.0f;
                float scaleY = windowSize.y / 900.0f;

                
                if (mouseX >= 927 * scaleX && mouseX <= 1277 * scaleX && mouseY >= 810 * scaleY && mouseY <= 889 * scaleY)
                {
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    currentScreen = "mainMenu";
                }
            }
        }
        window.clear();
        window.draw(background);
        window.draw(highScoresText);
        window.display();
    }
};

class PauseScreen : public Screen
{
public:
    PauseScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/pausesccreen.png", "pausescreen", 1);
        background.setTexture(rm.getTexture("pausescreen"));
    }
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        while (true)
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                {
                    currentScreen = "battlefield";
                    return;
                }
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    float mouseX = static_cast<float>(event.mouseButton.x);
                    float mouseY = static_cast<float>(event.mouseButton.y);
                    sf::Vector2u windowSize = window.getSize();
                    
                    
                    float scaleX = windowSize.x / 1300.0f;
                    float scaleY = windowSize.y / 900.0f;
                    
                    
                    if ((mouseX >= 377 * scaleX && mouseX <= 923 * scaleX) && (mouseY >= 318 * scaleY && mouseY <= 450 * scaleY))
                    {
                        SoundManager::getInstance()->playSound("buttonclick.wav");
                        currentScreen = "battlefield";
                        return;
                    }
                    
                    else if ((mouseX >= 205 * scaleX && mouseX <= 1095 * scaleX) && (mouseY >= 440 * scaleY && mouseY <= 617 * scaleY))
                    {
                        SoundManager::getInstance()->playSound("buttonclick.wav");
                        
                        StateManager::getInstance()->endBattleSession();
                        currentScreen = "mainMenu";
                        return;
                    }
                }
            }
            window.clear();
            window.draw(background);
            window.display();
        }
    }
};

class ProgressScreen : public Screen
{
public:
    sf::Clock clock;
    ProgressScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/levelup.png", "levelup", 1);
        background.setTexture(rm.getTexture("levelup"));
    }
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        if (clock.getElapsedTime().asSeconds() == 0) {
            clock.restart();
        }

        if (clock.getElapsedTime().asSeconds() > 3)
        {
            currentScreen = "battlefield";
            clock.restart();
            return;
        }
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        window.clear();
        window.draw(background);
        window.display();
    }
};

class GameOverScreen : public Screen
{
public:
    sf::Text gameOverText;
    sf::Text scoreText;
    sf::Text instructionText;
    sf::Font font;

    GameOverScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/endscreen.png", "endscreen", 1);
        background.setTexture(rm.getTexture("endscreen"));
        if (!rm.loadFont("assets/fonts/new.ttf", "arial")) {
            if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
                rm.loadFont("arial.ttf", "arial");
            }
        }
        gameOverText.setFont(rm.getFont("arial"));
        gameOverText.setCharacterSize(64);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setString("THE ZOMBIES ATE YOUR BRAINS!");
        gameOverText.setOrigin(gameOverText.getLocalBounds().width / 2, gameOverText.getLocalBounds().height / 2);
        gameOverText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 - 100);

        scoreText.setFont(rm.getFont("arial"));
        scoreText.setCharacterSize(48);
        scoreText.setFillColor(sf::Color::Yellow);
        scoreText.setPosition(50, GameConstants::BATTLEFIELD_HEIGHT / 2 - 20);

        instructionText.setFont(rm.getFont("arial"));
        instructionText.setCharacterSize(32);
        instructionText.setFillColor(sf::Color::White);
        instructionText.setString("Click anywhere to return to main menu");
        instructionText.setOrigin(instructionText.getLocalBounds().width / 2, instructionText.getLocalBounds().height / 2);
        instructionText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 + 50);
        
    }
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        scoreText.setString(std::to_string(player.score));
        scoreText.setPosition(50, GameConstants::BATTLEFIELD_HEIGHT / 2 - 20);
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                SoundManager::getInstance()->playSound("buttonclick.wav");
                
                StateManager::getInstance()->endBattleSession();
                currentScreen = "mainMenu";
            }
        }
        window.clear();
        window.draw(background);
        window.draw(gameOverText);
        window.draw(scoreText);
        window.draw(instructionText);
        window.display();
    }
};

class WinScreen : public Screen
{
public:
    sf::Text winText;
    sf::Text instructionText;
    sf::Font font;
    
    WinScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/levelup.png", "levelup", 1);
        background.setTexture(rm.getTexture("levelup"));
        if (!rm.loadFont("assets/fonts/new.ttf", "arial")) {
            if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
                rm.loadFont("arial.ttf", "arial");
            }
        }
        winText.setFont(rm.getFont("arial"));
        winText.setCharacterSize(64);
        winText.setFillColor(sf::Color::Green);
        winText.setString("YOU WIN!");
        winText.setOrigin(winText.getLocalBounds().width / 2, winText.getLocalBounds().height / 2);
        winText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 - 50);
        
        instructionText.setFont(rm.getFont("arial"));
        instructionText.setCharacterSize(32);
        instructionText.setFillColor(sf::Color::White);
        instructionText.setString("Click anywhere to continue");
        instructionText.setOrigin(instructionText.getLocalBounds().width / 2, instructionText.getLocalBounds().height / 2);
        instructionText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 + 50);
    }
    
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                SoundManager::getInstance()->playSound("buttonclick.wav");
                
                StateManager::getInstance()->endBattleSession();
                currentScreen = "mainMenu"; 
            }
        }
        window.clear();
        window.draw(background);
        window.draw(winText);
        window.draw(instructionText);
        window.display();
    }
};

class GameCompleteScreen : public Screen
{
public:
    sf::Text youWonText;
    sf::Text namePromptText;
    sf::Text inputText;
    sf::Text instructionText;
    sf::Font font;
    std::string playerName;
    bool namingMode;
    
    GameCompleteScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/endscreen.png", "endscreen", 1);
        background.setTexture(rm.getTexture("endscreen"));
        namingMode = true;
        
        if (!rm.loadFont("assets/fonts/new.ttf", "arial")) {
            if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
                rm.loadFont("arial.ttf", "arial");
            }
        }
        
        youWonText.setFont(rm.getFont("arial"));
        youWonText.setCharacterSize(72);
        youWonText.setFillColor(sf::Color::Yellow);
        youWonText.setString("YOU WON!");
        youWonText.setOrigin(youWonText.getLocalBounds().width / 2, youWonText.getLocalBounds().height / 2);
        youWonText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, 200);
        
        namePromptText.setFont(rm.getFont("arial"));
        namePromptText.setCharacterSize(36);
        namePromptText.setFillColor(sf::Color::White);
        namePromptText.setString("Enter your name:");
        namePromptText.setPosition(50, 400);
        
        inputText.setFont(rm.getFont("arial"));
        inputText.setCharacterSize(32);
        inputText.setFillColor(sf::Color::Green);
        inputText.setString("");
        inputText.setPosition(50, 450);
        
        instructionText.setFont(rm.getFont("arial"));
        instructionText.setCharacterSize(24);
        instructionText.setFillColor(sf::Color::White);
        instructionText.setString("Press ENTER to confirm");
        instructionText.setOrigin(instructionText.getLocalBounds().width / 2, instructionText.getLocalBounds().height / 2);
        instructionText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, 550);
        
        SoundManager::getInstance().playMusic("winmusic.wav", false);
    }
    
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::TextEntered && namingMode)
            {
                if (event.text.unicode == 8 && !playerName.empty())
                {
                    playerName.pop_back();
                }
                else if (event.text.unicode >= 32 && event.text.unicode <= 126 && playerName.length() < 20)
                {
                    playerName += static_cast<char>(event.text.unicode);
                }
                inputText.setString(playerName);
                inputText.setPosition(50, 450);
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter && namingMode)
            {
                if (!playerName.empty())
                {
                    std::ofstream scoreFile("highscores.txt", std::ios::app);
                    if (scoreFile.is_open())
                    {
                        scoreFile << playerName << " " << player.score << "\n";
                        scoreFile.close();
                    }
                    namingMode = false;
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    SoundManager::getInstance()->stopMusic();
                    SoundManager::getInstance()->setPlaylist(UIConstants::Audio::MENU_TRACKS);
                    currentScreen = "mainMenu";
                }
            }
        }
        
        window.clear();
        window.draw(background);
        window.draw(youWonText);
        if (namingMode)
        {
            window.draw(namePromptText);
            window.draw(inputText);
            window.draw(instructionText);
        }
        window.display();
    }
};

class LevelUpScreen : public Screen
{
public:
    sf::Clock clock;
    sf::Font font;
    sf::Text levelCompleteText;
    
    LevelUpScreen(sf::RenderWindow &window) : Screen(window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/levelup.png", "levelup", 1);
        background.setTexture(rm.getTexture("levelup"));
        
        if (!rm.loadFont("assets/fonts/new.ttf", "arial")) {
            if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
                rm.loadFont("arial.ttf", "arial");
            }
        }
        
        levelCompleteText.setFont(rm.getFont("arial"));
        levelCompleteText.setString("YOU WIN!");
        levelCompleteText.setCharacterSize(48);
        levelCompleteText.setFillColor(sf::Color::Green);
        levelCompleteText.setOrigin(levelCompleteText.getLocalBounds().width / 2, levelCompleteText.getLocalBounds().height / 2);
        levelCompleteText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2);
    }
    
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        
        clock.restart();
        while (clock.getElapsedTime().asSeconds() < 3) 
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
            }
            window.clear();
            window.draw(background);
            window.draw(levelCompleteText);
            window.display();
        }
        int currentLevel = StateManager::getInstance()->getLevel();
        LoadoutScreen::setCurrentLevel(currentLevel);
        currentScreen = "loadout"; 
    }
};

class BattlefieldScreen : public Screen
{
public:
    sf::Texture pauseButtonTexture;
    sf::Sprite pauseButton;
    GameCursor cursor;
    Shop sh;
    sf::Text currencyText;
    sf::Font font;
    sf::Text scoreText;
    sf::Text levelText;
    TowerFactory *towerFactory;
    EnemyFactory *enemyFactory;
    SunFactory *sunFactory;
    LawnMowerFactory *lawnMowerFactory;
    DefenseSynth::Inspector inspector;
    CollisionManager collisionManager;
    std::map<std::string, int> unitUpgradeLevels;
    float gameDifficulty;
    bool gameStarted = false;
    int currentWave = 0;
    bool waveInProgress = false;
    sf::Clock waveTimer;
    sf::Text waveText;
    bool showWaveText = false;
    sf::Clock waveTextTimer;
    sf::Text cityPopulationText;
    int cityPopulation; 

    BattlefieldScreen(sf::RenderWindow &window, const std::vector<std::string>& allowedUnits, const std::map<std::string, int>& upgradeLevels, float difficulty = 1.0f) : Screen(window)
    {
        gameStarted = false;
        currentWave = 0;
        waveInProgress = false;
        showWaveText = false;
        unitUpgradeLevels = upgradeLevels;
        for (const auto& pair : upgradeLevels) {
            StateManager::getInstance()->setUpgradeLevel(pair.first, pair.second);
        }
        gameDifficulty = difficulty;
        
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        if (!rm.loadFont("assets/fonts/new.ttf", "arial")) {
            if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
                rm.loadFont("arial.ttf", "arial");
            }
        }
        rm.loadTextureWithRetry("assets/images/background.png", "background", 1);
        background.setTexture(rm.getTexture("background"));
        
        waveText.setFont(rm.getFont("arial"));
        waveText.setCharacterSize(48);
        waveText.setFillColor(sf::Color::Red);
        waveText.setStyle(sf::Text::Bold);
        waveText.setOrigin(waveText.getLocalBounds().width / 2, waveText.getLocalBounds().height / 2);
        waveText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2);

        currencyText.setFont(rm.getFont("arial"));
        currencyText.setCharacterSize(24);
        currencyText.setFillColor(sf::Color::Black);
        currencyText.setPosition(45, 70);

        scoreText.setFont(rm.getFont("arial"));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(1100, 850);

        levelText.setFont(rm.getFont("arial"));
        levelText.setCharacterSize(24);
        levelText.setFillColor(sf::Color::Red);
        levelText.setPosition(20, 850);
        
        cityPopulationText.setFont(rm.getFont("arial"));
        cityPopulationText.setCharacterSize(28);
        cityPopulationText.setFillColor(sf::Color::Blue);
        cityPopulationText.setStyle(sf::Text::Bold);
        cityPopulationText.setPosition(20, 820);
        cityPopulation = 100;

        rm.loadTextureWithRetry("assets/images/pause.png", "pause", 1);
        pauseButton.setTexture(rm.getTexture("pause"));
        pauseButton.setPosition(1200, 0);

        
        towerFactory = new TowerFactory();
        sunFactory = new SunFactory(10);
        enemyFactory = new EnemyFactory(8, 1, gameDifficulty);
        lawnMowerFactory = new LawnMowerFactory(5);
        
        
        sh.setAllowedUnits(allowedUnits);
        cursor.setAllowedUnits(allowedUnits);
        
        #ifdef _DEBUG
        std::cout << "BattlefieldScreen constructor - Setting " << allowedUnits.size() << " allowed units in shop:" << std::endl;
        for (const auto& unit : allowedUnits) {
            std::cout << "  - " << unit << std::endl;
        }
        #endif
        
        
        StateManager* sm = StateManager::getInstance();
        player.level = sm->getLevel();
        player.currency = 100; 
        player.lives = 3;     
     
        
        
        delete enemyFactory;
        delete sunFactory;
        enemyFactory = new EnemyFactory(player.level * 6, player.level, gameDifficulty);
        sunFactory = new SunFactory(player.level * 20);
        
        
        SoundManager::getInstance()->setPlaylist(UIConstants::Audio::BATTLE_TRACKS);
    }

    ~BattlefieldScreen()
    {
        delete towerFactory;
        delete enemyFactory;
        delete sunFactory;
        delete lawnMowerFactory;
    }

    void startNextLevel()
    {
        
        
        enemyFactory->resetForNextLevel(4 + player.level * 2, player.level, gameDifficulty);
        sunFactory->resetForNextLevel(player.level * 20);
        lawnMowerFactory->resetForNextLevel(5);
        
        player.currency += 50;
    }

    void Serialize()
    {
        std::ofstream saveFile("save/game_save.dat", std::ios::trunc);
        if (saveFile.is_open())
        {
            player.Serialize(saveFile);
            saveFile.close();
        }
    }

    void Deserialize()
    {
        std::ifstream stream("save/game_save.dat");
        if (stream.is_open())
        {
            player.Deserialize(stream);
            stream.close();
        }
    }

    void showGameOverScreen(sf::RenderWindow &window)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        std::string playerName;
        sf::Text playerNameText;
        sf::Text scoreText;
        
        playerNameText.setFont(rm.getFont("arial"));
        playerNameText.setCharacterSize(50);
        playerNameText.setFillColor(sf::Color::White);
        playerNameText.setPosition(695, 760);
        
        scoreText.setFont(rm.getFont("arial"));
        scoreText.setCharacterSize(50);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setString(std::to_string(player.score));
        scoreText.setOrigin(
            scoreText.getLocalBounds().left + scoreText.getLocalBounds().width / 2,
            scoreText.getLocalBounds().top + scoreText.getLocalBounds().height / 2);
        scoreText.setPosition(741, 480);
        
        sf::Sprite endSprite;
        rm.loadTextureWithRetry("assets/images/endscreen.png", "endscreen", 1);
        endSprite.setTexture(rm.getTexture("endscreen"));
        endSprite.setPosition(0, 0);
        

        while (true)
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
                else if (event.type == sf::Event::TextEntered)
                {
                    if (event.text.unicode == '\b')
                    {
                        if (!playerName.empty())
                            playerName.pop_back();
                    }
                    else if (event.text.unicode == '\r')
                    {
                        if (!playerName.empty())
                        {
                            std::ofstream outFile("highscores.txt", std::ios::app);
                            if (outFile.is_open())
                            {
                                outFile << playerName << " " << player.score << "\n";
                                outFile.close();
                            }
                            currentScreen = "mainMenu";
                            return;
                        }
                    }
                    else if (event.text.unicode < 128)
                    {
                        playerName += static_cast<char>(event.text.unicode);
                    }
                }
                else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    currentScreen = "mainMenu";
                    return;
                }
            }

            window.clear();
            window.draw(endSprite);
            window.draw(playerNameText);
            window.draw(scoreText);
            window.display();
        }
    }

    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        SoundManager::getInstance()->updateMusic();
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                
                StateManager* sm = StateManager::getInstance();
                sm->setLevel(player.level);
                sm->setCrystals(CrystalManager::getInstance().getCrystalCount());
                sm->save();
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P)
                {
                    SoundManager::getInstance()->playSound("pause.wav");
                    currentScreen = "pause";
                    return;
                }
                else if (event.key.code == sf::Keyboard::Space && !gameStarted)
                {
                    gameStarted = true;
                    
                    StateManager::getInstance()->startBattleSession();
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2u windowSize = window.getSize();
                float scaleX = windowSize.x / 1300.0f;
                float scaleY = windowSize.y / 900.0f;

                
                sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

                coordinates clickPosition;
                clickPosition.x = worldPos.x;
                clickPosition.y = worldPos.y;

                
                if (pauseButton.getGlobalBounds().contains(worldPos))
                {
                    SoundManager::getInstance()->playSound("buttonclick.wav");
                    currentScreen = "pause";
                    return;
                }
                else
                {
                    inspector.collectSun(clickPosition, sunFactory, towerFactory, player);
                    inspector.addTowerOnClick(clickPosition, towerFactory, cursor, player, &unitUpgradeLevels);
                    cursor.renderCursor(clickPosition, player);
                }
            }
        }

        if (gameStarted) {
            if (currentWave == 0) {
                currentWave = 1;
                waveInProgress = true;
                waveTimer.restart();
                showWaveText = true;
                waveText.setString("Wave 1: Normal Assault!");
                waveText.setOrigin(waveText.getLocalBounds().width / 2, waveText.getLocalBounds().height / 2);
                waveTextTimer.restart();
                
                int wave1EnemiesActivated = 0;
                int wave1TargetCount = 0;
                for (int i = 0; i < enemyFactory->enemies_created; i++) {
                    if (enemyFactory->enemies[i]->type.find("boss_") != 0) {
                        wave1TargetCount++;
                    }
                }
                wave1TargetCount = wave1TargetCount / 2;
                
                for (int i = 0; i < enemyFactory->enemies_created && wave1EnemiesActivated < wave1TargetCount; i++) {
                    if (!enemyFactory->enemies[i]->enabled && enemyFactory->enemies[i]->type.find("boss_") != 0) {
                        enemyFactory->enemies[i]->enabled = true;
                        wave1EnemiesActivated++;
                    }
                }
            }
            
            if (showWaveText && waveTextTimer.getElapsedTime().asSeconds() > 3.0f) {
                showWaveText = false;
            }
            
            if (waveInProgress) {
                if (currentWave == 1) {
                    bool allNormalEnemiesDead = true;
                    for (int i = 0; i < enemyFactory->enemies_created; i++) {
                        if (enemyFactory->enemies[i]->enabled && enemyFactory->enemies[i]->type.find("boss_") != 0) {
                            allNormalEnemiesDead = false;
                            break;
                        }
                    }
                    if (allNormalEnemiesDead) {
                        currentWave = 2;
                        waveInProgress = true;
                        waveTimer.restart();
                        showWaveText = true;
                        waveText.setString("Wave 2: Boss Invasion!");
                        waveText.setOrigin(waveText.getLocalBounds().width / 2, waveText.getLocalBounds().height / 2);
                        waveTextTimer.restart();
                        
                        for (int i = 0; i < towerFactory->towers_created; i++) {
                            if (towerFactory->towers[i]->category == "AttackTower") {
                                towerFactory->towers[i]->clock.restart();
                            }
                        }
                        
                        enemyFactory->triggerBossWave();
                        
                        for (size_t i = 0; i < enemyFactory->enemies.size(); i++) {
                            if (enemyFactory->enemies[i] && enemyFactory->enemies[i]->type.find("boss_") == 0) {
                                enemyFactory->enemies[i]->enabled = true;
                            }
                        }
                    }
                } else if (currentWave == 2) {
                    bool anyBossAlive = false;
                    for (size_t i = 0; i < enemyFactory->enemies.size(); ++i) {
                        if (enemyFactory->enemies[i] && enemyFactory->enemies[i]->enabled &&
                            enemyFactory->enemies[i]->type.find("boss_") == 0) {
                            anyBossAlive = true;
                            break;
                        }
                    }
                    if (!anyBossAlive) {
                        waveInProgress = false;
                        gameStarted = false;
                        currentLevel++;
                        
                        player->levelUp();
                        towerFactory->setPlayer(player);
                        
                        if (currentLevel >= 10) {
                            screenState = STATE::WIN_SCREEN;
                        } else {
                            screenState = STATE::LEVELUP_SCREEN;
                        }
                    }
                }
            }
            
            enemyFactory->summonEnemies();
            sunFactory->move();
            lawnMowerFactory->move();
            enemyFactory->move();
            towerFactory->move();
        }
        
        
        for (int i = 0; i < towerFactory->towers_created; i++) {
            if (towerFactory->towers[i]->type == "ResourceGenerator") {
                ResourceGenerator* resourceGen = static_cast<ResourceGenerator*>(towerFactory->towers[i]);
                resourceGen->updateBullet();
            }
        }

        collisionManager.checkCollisions(towerFactory, enemyFactory, player);
        collisionManager.checkLawnmowerCollisions(lawnMowerFactory, enemyFactory, player);
        
        
        towerFactory->removeInactiveTowers();
        
        inspector.hasEnemyReachedEdge(enemyFactory, player, cityPopulation);

            
            if (cityPopulation <= 0)
            {
                StateManager::getInstance()->endBattleSession();
                StateManager::getInstance()->save();
                
                showGameOverScreen(window);
                currentScreen = "mainMenu";
                return;
            }

            
            if (!waveInProgress && currentWave == 2)
            {
                int completedLevel = player.level;
                player.level++;
                
                if (player.level >= 10)
                {
                    currentScreen = "gameComplete";
                    return;
                }
                
                StateManager* sm = StateManager::getInstance();
                sm->setLevel(player.level);
                int newMaxSelection = std::min(3 + player.level - 1, 7);
                sm->setMaxSelection(newMaxSelection);
                
                CrystalManager& crystalManager = CrystalManager::getInstance();
                int crystalReward = crystalManager.getLevelCompletionReward(completedLevel);
                crystalManager.addCrystals(crystalReward);
                sm->setCrystals(crystalManager.getCrystalCount());
                sm->save();
                
                delete enemyFactory;
                delete towerFactory;
                delete sunFactory;
                delete lawnMowerFactory;
                
                enemyFactory = new EnemyFactory(player.level * 6, player.level, gameDifficulty);
                towerFactory = new TowerFactory();
                sunFactory = new SunFactory(player.level * 20);
                lawnMowerFactory = new LawnMowerFactory(5);
                
                player.currency = 0;
                gameStarted = false;
                currentWave = 0;
                waveInProgress = false;
                currentScreen = "levelUp";
                return;
            }

        window.clear();
        window.draw(background);
        
        enemyFactory->drawGroundEnemies(window);
        towerFactory->drawSprites(window);
        
        for (int i = 0; i < towerFactory->towers_created; i++) {
            if (towerFactory->towers[i]->category == "AttackTower") {
                AttackTower* attacker = static_cast<AttackTower*>(towerFactory->towers[i]);
                attacker->drawBullet(window);
            }
        }
        
        for (int i = 0; i < towerFactory->towers_created; i++) {
            if (towerFactory->towers[i]->type == "ResourceGenerator") {
                ResourceGenerator* resourceGen = static_cast<ResourceGenerator*>(towerFactory->towers[i]);
                resourceGen->drawBullet(window);
            }
        }
        
        lawnMowerFactory->draw(window);
        enemyFactory->drawFlyingEnemies(window);
        
        if (!gameStarted) {
            sf::Text startText;
            startText.setFont(font);
            startText.setString("Press SPACE to Start Battle!");
            startText.setCharacterSize(36);
            startText.setFillColor(sf::Color::Yellow);
            startText.setStyle(sf::Text::Bold);
            startText.setOrigin(startText.getLocalBounds().width / 2, startText.getLocalBounds().height / 2);
            startText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2);
            window.draw(startText);
        }
        
        
        for (int i = 0; i < towerFactory->towers_created; i++) {
            if (towerFactory->towers[i]->operational) {
                Tower* tower = towerFactory->towers[i];
                tower->update(0.016f);
                
                float healthPercentage = static_cast<float>(tower->vitality) / static_cast<float>(tower->maxVitality);
                healthPercentage = std::max(0.0f, std::min(1.0f, healthPercentage));
                
                sf::RectangleShape healthBarBg(sf::Vector2f(8, 42));
                healthBarBg.setFillColor(sf::Color::Black);
                healthBarBg.setOutlineThickness(1);
                healthBarBg.setOutlineColor(sf::Color::White);
                healthBarBg.setPosition(tower->location.x - 16, tower->location.y + 8);
                window.draw(healthBarBg);
                
                float barHeight = 40 * healthPercentage;
                sf::RectangleShape healthBar(sf::Vector2f(6, barHeight));
                
                sf::Color healthColor;
                if (healthPercentage > 0.6f) {
                    healthColor = sf::Color::Green;
                } else if (healthPercentage > 0.3f) {
                    healthColor = sf::Color::Yellow;
                } else {
                    healthColor = sf::Color::Red;
                }
                healthBar.setFillColor(healthColor);
                healthBar.setPosition(tower->location.x - 15, tower->location.y + 9 + (40 - barHeight));
                window.draw(healthBar);
                
                if (healthPercentage < 1.0f) {
                    float damageHeight = 40 * (1.0f - healthPercentage);
                    sf::RectangleShape damageBar(sf::Vector2f(6, damageHeight));
                    damageBar.setFillColor(sf::Color(80, 80, 80, 180));
                    damageBar.setPosition(tower->location.x - 15, tower->location.y + 9);
                    window.draw(damageBar);
                }
            }
        }
        
        enemyFactory->drawLabels(window);
        
        std::string currencyStr = std::to_string(player.currency);
        currencyText.setString(currencyStr);
        sf::FloatRect currencyBounds = currencyText.getLocalBounds();
        currencyText.setPosition(std::max(25.0f, 60.0f - currencyBounds.width), 70);
        scoreText.setString("Score: " + std::to_string(player.score));
        levelText.setString("Level: " + std::to_string(player.level));
        
        cityPopulationText.setString("Citizens: " + std::to_string(cityPopulation));
        
        window.draw(scoreText);
        window.draw(levelText);
        window.draw(cityPopulationText);
        window.draw(pauseButton);
        
        if (showWaveText) {
            window.draw(waveText);
        }
        
        sunFactory->draw(window);
        sh.draw(window, player, player.level);
        window.draw(currencyText);
        
        cursor.applyCursor(window);
        window.display();
    }
};

class LoadoutScreenWrapper : public Screen
{
private:
    LoadoutScreen* loadoutScreen;
    CardCollection cardCollection;
    std::vector<std::string> cachedSelection;
    std::map<std::string, int> cachedUpgradeLevels;
    
    
public:
    LoadoutScreenWrapper(sf::RenderWindow &window) : Screen(window)
    {
        
        StateManager* sm = StateManager::getInstance();
        int currentLevel = sm->getLevel();
        
        sm->clearUnlockedCards();
        
        sm->unlockCard("basicshooter");
        sm->unlockCard("resourcegenerator");
        sm->unlockCard("barriertower");
        sm->unlockCard("rapidshooter");
        
        if (currentLevel >= 2) {
            sm->unlockCard("freezetower");
        }
        if (currentLevel >= 3) {
            sm->unlockCard("bombtower");
        }
        if (currentLevel >= 4) {
            sm->unlockCard("areaattacktower");
        }
        
        
        CrystalManager::getInstance().setCrystalCount(sm->getCrystals());
        
        
        cardCollection.loadProgressFromFile();
        
        loadoutScreen = new LoadoutScreen(window, cardCollection);
    }
    
    ~LoadoutScreenWrapper() {
        delete loadoutScreen;
    }
    
    
    void refreshFromStateManager(sf::RenderWindow &window) {
        StateManager* sm = StateManager::getInstance();
        
        
        CrystalManager::getInstance().setCrystalCount(sm->getCrystals());
        
        
        cardCollection.loadProgressFromFile();
        
        
        delete loadoutScreen;
        loadoutScreen = new LoadoutScreen(window, cardCollection);
    }
    
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen)
    {
        
        refreshFromStateManager(window);
        
        std::string currentView = "loadout";
        loadoutScreen->renderScreen(window, currentView);
        
        if (currentView == "battlefield") {
            
            cachedSelection = loadoutScreen->getSelectedUnits();
            
            #ifdef _DEBUG
            std::cout << "LoadoutScreenWrapper::renderScreen - Caching " << cachedSelection.size() << " selected units:" << std::endl;
            for (const auto& unit : cachedSelection) {
                std::cout << "  - " << unit << std::endl;
            }
            #endif
            
            
            cachedUpgradeLevels.clear();
            for (const std::string& unitType : cachedSelection) {
                UnitCard* card = cardCollection.getCard(unitType);
                int level = card ? card->level : 1;
                cachedUpgradeLevels[unitType] = level;
            }
            
            
            StateManager::getInstance()->setSelectedUnits(cachedSelection);
            StateManager::getInstance()->save();
            
            
            StateManager* sm = StateManager::getInstance();
            for (const auto& card : cardCollection.getAllCards()) {
                sm->setCardLevel(card.unitType, card.level);
                if (card.unlocked) {
                    sm->unlockCard(card.unitType);
                }
            }
            currentScreen = "battlefield";
        } else if (currentView == "mainMenu") {
            currentScreen = "mainMenu";
        }
    }

    const std::vector<std::string> getSelectedUnits() const { 
        std::vector<std::string> actualSelection = StateManager::getInstance()->getSelectedUnits();
        
        #ifdef _DEBUG
        std::cout << "LoadoutScreenWrapper::getSelectedUnits() - Returning " << actualSelection.size() << " units:" << std::endl;
        for (const auto& unit : actualSelection) {
            std::cout << "  - " << unit << std::endl;
        }
        #endif
        
        return actualSelection;
    }
    
    std::map<std::string, int> getUpgradeLevels() const { 
        std::map<std::string, int> actualUpgradeLevels;
        std::vector<std::string> selectedUnits = getSelectedUnits();
        
        for (const std::string& unitType : selectedUnits) {
            UnitCard* card = const_cast<CardCollection&>(cardCollection).getCard(unitType);
            int level = card ? card->level : 1;
            actualUpgradeLevels[unitType] = level;
            StateManager::getInstance()->setUpgradeLevel(unitType, level);
        }
        
        #ifdef _DEBUG
        std::cout << "LoadoutScreenWrapper::getUpgradeLevels() - Returning " << actualUpgradeLevels.size() << " upgrades:" << std::endl;
        for (const auto& pair : actualUpgradeLevels) {
            std::cout << "  - " << pair.first << ": level " << pair.second << std::endl;
        }
        #endif
        
        return actualUpgradeLevels;
    }
    
    int getUpgradeLevel(const std::string& unitType) const {
        auto upgradeLevels = getUpgradeLevels();
        auto it = upgradeLevels.find(unitType);
        return (it != upgradeLevels.end()) ? it->second : 1;
    }
    
    float getDifficultyValue() const {
        return loadoutScreen ? loadoutScreen->getDifficultyValue() : 1.0f;
    }
};
