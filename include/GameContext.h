#pragma once

#include "GamePlayer.h"

namespace DefenseSynth {

struct GameSettings {
    int windowWidth = 1300;
    int windowHeight = 900;
    int targetFPS = 60;
    float musicVolume = 50.0f;
    float sfxVolume = 75.0f;
};

class GameContext {
private:
    Player gamePlayer;
    int currentLevel;
    float difficultyMultiplier;
    GameSettings settings;
    static GameContext* instance;

public:
    static GameContext* getInstance() {
        if (instance == nullptr) {
            instance = new GameContext();
        }
        return instance;
    }
    
    static void cleanup() {
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }
    }
    
    Player& getPlayer() { return gamePlayer; }
    const Player& getPlayer() const { return gamePlayer; }
    
    int getCurrentLevel() const { return currentLevel; }
    float getDifficulty() const { return difficultyMultiplier; }
    GameSettings& getSettings() { return settings; }
    const GameSettings& getSettings() const { return settings; }
    
    void setLevel(int level) { currentLevel = level; }
    void setDifficulty(float diff) { difficultyMultiplier = diff; }
    
    void reset() {
        gamePlayer.reset();
        currentLevel = 1;
        difficultyMultiplier = 1.0f;
    }
    
private:
    GameContext() {
        gamePlayer.level = 1;
        gamePlayer.currency = 100;
        gamePlayer.lives = 3;
        gamePlayer.score = 0;
        currentLevel = 1;
        difficultyMultiplier = 1.0f;
    }
    
    ~GameContext() = default;
    
    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;
};


}
