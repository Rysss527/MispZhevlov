#pragma once

#include "../GamePlayer.h"
#include "../EnemyFactory.h"
#include "../TowerFactory.h"

namespace DefenseSynth {

enum class GameState {
    InProgress,
    Won,
    Lost,
    Paused
};

struct WaveInfo {
    int waveNumber;
    int totalEnemies;
    int enemiesSpawned;
    int enemiesKilled;
    bool isComplete;
};

class GameStateValidator {
public:
    GameStateValidator();
    ~GameStateValidator() = default;
    
    bool hasPlayerWon() const;
    bool hasPlayerLost() const;
    void checkEndConditions(Player& player, EnemyFactory* enemies);
    
    GameState getCurrentState() const { return currentState; }
    void setState(GameState state) { currentState = state; }
    
    bool isWaveComplete(EnemyFactory* enemies) const;
    bool areAllEnemiesDefeated(EnemyFactory* enemies) const;
    bool hasEnemyReachedEnd(EnemyFactory* enemies) const;
    
    void startWave(int waveNumber, int enemyCount);
    void endWave();
    WaveInfo getCurrentWaveInfo() const { return currentWave; }
    
    int getCityPopulation() const { return cityPopulation; }
    void setCityPopulation(int population) { cityPopulation = population; }
    void decreaseCityPopulation(int amount);
    
    bool shouldSpawnNextWave() const;
    float getTimeSinceWaveEnd() const;
    
    void updateWaveProgress(EnemyFactory* enemies);
    void resetForNewGame();
    
private:
    GameState currentState;
    WaveInfo currentWave;
    int cityPopulation;
    int maxCityPopulation;
    bool waveInProgress;
    sf::Clock waveEndClock;
    
    static constexpr int INITIAL_CITY_POPULATION = 100;
    static constexpr float WAVE_INTERVAL = 5.0f;
    static constexpr float LEFT_BOUNDARY = 100.0f;
};

}
